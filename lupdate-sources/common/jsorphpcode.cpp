#include "jsorphpcode.h"

QList<TsFile::Translation> JsOrPhpCode::translations() const{
    return this->_translations;
}

QString JsOrPhpCode::lastError() const{
    return "Line " + QString::number(this->lineNumber(this->_lastErrorPos)) + ": " + this->_lastError;
}

bool JsOrPhpCode::parse(){
    this->_translations.clear();

    for(const auto &[codePortion, offset]: this->codePortions()){
        StringType currentStringType;
        QString currentStringDelimiter;
        QString currentString;

        constexpr const int NOT_IN_TR_FUNCTION = 0,
                            BEFORE_FIRST_ARG = 1,
                            AFTER_FIRST_ARG = 2,
                            BEFORE_SECOND_ARG = 3,
                            AFTER_SECOND_ARG = 4;
        int trArgPosition = NOT_IN_TR_FUNCTION;
        QList<int> templateLiteralDepth;    //The key represents how many template literals are in, and the value represents how many curly braces we have. So for example if we're `${if(true){`${here}`}}`, it will be [1, 0]: we're inside 1 curly brace in the outer remplate literal and 0 in the inner template literal.
        TsFile::Translation currentTranslation;

        //Sometimes mid(i, 1) is used instead of [i] in case it's out of bounds
        for(int i = 0; i < codePortion.size(); i++){
            //Inside of strings
            if(!currentStringDelimiter.isEmpty()){
                if(codePortion[i] == '\\' && (!currentStringType.literalBackslashOnMissingEscapeSequence || currentStringType.escapeSequences.contains(codePortion.mid(i + 1, 1)))){
                    i++;
                    if(i >= codePortion.size()){
                        this->_lastError = "Unterminated string literal.";
                        this->_lastErrorPos = offset + i;
                        return false;
                    }
                    const char characterAfterBackslash = codePortion[i].toLatin1();
                    switch(characterAfterBackslash){
                    case 'n':
                        currentString.append('\n');
                        break;
                    case 'r':
                        currentString.append('\r');
                        break;
                    case 'v':
                        currentString.append('\v');
                        break;
                    case 't':
                        currentString.append('\t');
                        break;
                    case 'b':
                        currentString.append('\b');
                        break;
                    case 'e':
                        currentString.append('\x1B');
                        break;
                    case 'f':
                        currentString.append('\f');
                        break;
                    case '\n':
                        //Don't do anything, a backslash followed by a newline is used to ignore the newline
                        break;
                    case 'x':
                    case 'u':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':{
                        const bool isOctal = characterAfterBackslash >= '0' && characterAfterBackslash <= '7';
                        QString unicodeSequence;
                        if(characterAfterBackslash == 'x'){
                            //Hexadecimal escape sequence, for example \xA9
                            unicodeSequence = codePortion.mid(i + 1, 2);
                            i += 2;
                        }
                        else if(isOctal){
                            //Octal escape sequence, for example \251
                            for(; i < codePortion.size() && codePortion[i] >= '0' && codePortion[i] <= '7' && unicodeSequence.size() < 3; i++){
                                unicodeSequence.append(codePortion[i]);
                            }
                        }
                        else if(codePortion.mid(i + 1, 1) == "{"){
                            //Unicode code point escape, for example \u{A9}
                            for(i += 2; i < codePortion.size() && codePortion[i] != '}'; i++){
                                unicodeSequence.append(codePortion[i]);
                            }
                        }
                        else{
                            //Unicode escape sequence, for example \u00A9
                            unicodeSequence = codePortion.mid(i + 1, 4);
                            i += 4;
                        }
                        bool ok;
                        int charCode = unicodeSequence.toInt(&ok, isOctal ? 8 : 16);
                        if(!ok){
                            if(codePortion.mid(i, 1) == "}"){
                                unicodeSequence = "{" + unicodeSequence + "}";
                            }
                            this->_lastError = "Invalid hexadecimal escape sequence \\" + (characterAfterBackslash + unicodeSequence) + ".";
                            this->_lastErrorPos = offset + i;
                            return false;
                        }
                        if(isOctal){
                            charCode %= 256;
                        }
                        currentString.append(QChar(charCode));
                        break;
                    }
                    case '\'':
                    case '"':
                    case '`':
                    case '\\':
                    case '$':
                    default:
                        currentString.append(characterAfterBackslash);
                        break;
                    }
                }
                else if(codePortion.mid(i, currentStringDelimiter.size()) == currentStringDelimiter){
                    static const QRegularExpression disallowedCharacters("[\\0-\x08\x0b\x0c\x0e-\x1f]");
                    if((trArgPosition == BEFORE_FIRST_ARG || trArgPosition == BEFORE_SECOND_ARG) && currentString.contains(disallowedCharacters)){
                        this->_lastError = "ASCII control characters other than tabs and newlines are not allowed as arguments to QObject.tr() function.";
                        this->_lastErrorPos = offset + i;
                        return false;
                    }
                    switch(trArgPosition){
                    case BEFORE_FIRST_ARG:
                        currentTranslation.sourceText = currentString;
                        trArgPosition = AFTER_FIRST_ARG;
                        break;
                    case BEFORE_SECOND_ARG:
                        currentTranslation.disambiguation = currentString;
                        trArgPosition = AFTER_SECOND_ARG;
                        break;
                    }
                    i += currentStringDelimiter.size() - 1;
                    currentStringDelimiter.clear();
                    currentString.clear();
                }
                else if(trArgPosition != NOT_IN_TR_FUNCTION && currentStringType.supportsInterpolation && codePortion[i] == '$' && (currentStringDelimiter != "`" || codePortion.mid(i + 1, 1) == "{")){
                    if(currentStringDelimiter == "`"){
                        this->_lastError = "Template literals that use the ${someVariable} syntax are not allowed as arguments for QObject.tr.";
                    }
                    else{
                        this->_lastError = "Double-quoted strings or heredocs that use the $someVariable syntax are not allowed as arguments for QObject::tr.";
                    }
                    this->_lastErrorPos = offset + i;
                    return false;
                }
                else if(currentStringDelimiter == "`" && codePortion.mid(i, 2) == "${"){
                    templateLiteralDepth.append(0);
                    currentStringDelimiter.clear();
                    currentString.clear();
                    i++;    //It will be automatically incremented once by the for loop, but it needs to be incremented twice because there are two characters in "${"
                }
                else{
                    currentString.append(codePortion[i]);

                    //Remove indentation from heredoc/nowdoc strings
                    const int leadingWhitespaceInDelimiter = currentStringDelimiter.size() - currentStringDelimiter.trimmed().size();
                    if(leadingWhitespaceInDelimiter > 1 && codePortion.mid(i, leadingWhitespaceInDelimiter) == currentStringDelimiter.first(leadingWhitespaceInDelimiter)){
                        i += leadingWhitespaceInDelimiter - 1;
                    }
                }
                continue;
            }
            else if(templateLiteralDepth.size() > 0){
                if(codePortion[i] == '}'){
                    if(templateLiteralDepth.last() == 0){
                        templateLiteralDepth.removeLast();
                        currentStringDelimiter = "`";
                    }
                    else{
                        templateLiteralDepth.last()--;
                    }
                    continue;
                }
                else if(codePortion[i] == '{'){
                    templateLiteralDepth.last()++;
                    continue;
                }
            }

            //Start of strings
            for(const StringType &stringType: this->stringTypes()){
                if(codePortion.mid(i, stringType.delimiter.size()) == stringType.delimiter){
                    if(stringType.delimiter.startsWith("<<<")){
                        i += 3;
                        const bool isNowdoc = codePortion.mid(i, 1) == "'";
                        if(isNowdoc){
                            i++;
                        }
                        const QRegularExpressionMatch match = this->identifier().match(codePortion, i);
                        if(!match.hasMatch()){
                            this->_lastError = "Invalid character after <<< token.";
                            this->_lastErrorPos = offset + i;
                            return false;
                        }
                        if(isNowdoc){
                            i++;
                        }
                        const QRegularExpression stringEndRegex("\n[^\\S\n]*" + QRegularExpression::escape(match.captured()));
                        const QRegularExpressionMatch stringEndMatch = stringEndRegex.match(codePortion, i);
                        if(!stringEndMatch.hasMatch()){
                            if(isNowdoc){
                                this->_lastError = "Unterminated nowdoc string.";
                            }
                            else{
                                this->_lastError = "Unterminated heredoc string.";
                            }
                            this->_lastErrorPos = offset + i;
                            return false;
                        }
                        currentStringDelimiter = stringEndMatch.captured(0);
                    }
                    else{
                        currentStringDelimiter = stringType.delimiter;
                    }
                    currentStringType = stringType;
                    break;
                }
            }
            if(!currentStringDelimiter.isEmpty()){
                i += currentStringDelimiter.size() - 1;
                continue;
            }

            //Comments
            const QString twoNextChars = codePortion.mid(i, 2);
            if(twoNextChars == "//" || (this->hasShellStyleComments() && codePortion[i] == '#')){
                i = codePortion.indexOf('\n', i + 1);
                if(i == -1){
                    break;
                }
                continue;
            }
            else if(twoNextChars == "/*"){
                i = codePortion.indexOf("*/", i + 2);
                if(i == -1){
                    this->_lastError = "Unterminated multiline comment.";
                    this->_lastErrorPos = offset + i;
                    return false;
                }
                continue;
            }

            //If we're in a tr() call but we get something else than a comment, a space or a string literal, throw an error unless it's a , or )
            //Only check for spaces, the rest is done above
            if(trArgPosition != NOT_IN_TR_FUNCTION && !codePortion[i].isSpace()){
                switch(trArgPosition){
                case AFTER_FIRST_ARG:
                    if(codePortion[i] == ','){
                        trArgPosition = BEFORE_SECOND_ARG;
                        continue;
                    }
                    else if(codePortion[i] == ')'){
                        trArgPosition = NOT_IN_TR_FUNCTION;
                        currentTranslation.disambiguation.clear();
                        currentTranslation.hasPlural = false;
                        this->_translations.append(currentTranslation);
                        continue;
                    }
                    //fall through
                case BEFORE_FIRST_ARG:
                    this->_lastError = "First argument of QObject.tr() must be a string literal.";
                    this->_lastErrorPos = offset + i;
                    return false;
                case AFTER_SECOND_ARG:
                    if(codePortion[i] == ',' || codePortion[i] == ')'){
                        trArgPosition = NOT_IN_TR_FUNCTION;
                        currentTranslation.hasPlural = codePortion[i] == ',';
                        this->_translations.append(currentTranslation);
                        continue;
                    }
                    //fall through
                case BEFORE_SECOND_ARG:
                    if(codePortion.mid(i, 4) == "null"){
                        i += 3;
                        trArgPosition = AFTER_SECOND_ARG;
                        continue;
                    }
                    this->_lastError = "Second argument of QObject.tr() must be a string literal.";
                    this->_lastErrorPos = offset + i;
                    return false;
                }
            }

            //Calls to the tr() function
            const QRegularExpressionMatch trMatch = this->trFunction().match(codePortion.mid(i));
            if(trMatch.hasMatch()){
                trArgPosition = BEFORE_FIRST_ARG;
                currentTranslation.locations = {{this->filePath(), this->lineNumber(offset + i)}};
                i += trMatch.capturedLength() - 1;
            }
        }
    }
    return true;
}
