#include "phpcode.h"

QList<QPair<QString, int>> PhpCode::codePortions() const{
    QList<QPair<QString, int>> result;
    static const QRegularExpression openingPhpTag("<\\?(php|=)?"), closingPhpTag("\\?>");
    const QString contents = this->contents();
    QRegularExpressionMatchIterator matches = openingPhpTag.globalMatch(contents);
    for(const QRegularExpressionMatch &startMatch: matches){
        const int offset = startMatch.capturedEnd();
        const QRegularExpressionMatch endMatch = closingPhpTag.match(contents, offset);
        const QString codePortion = contents.mid(offset, endMatch.hasMatch() ? endMatch.capturedStart() - offset : -1);
        result.append({codePortion, offset});
    }
    return result;
}

bool PhpCode::hasShellStyleComments() const{
    return true;
}

QList<JsOrPhpCode::StringType> PhpCode::stringTypes() const{
    //List of escape sequences in PHP strings: https://www.php.net/manual/en/language.types.string.php#language.types.string.syntax.double
    const QStringList escapeSequences{"0", "1", "2", "3", "4", "5", "6", "7", "n", "r", "t", "v", "e", "f", "\\", "$", "x", "u"};
    return {
        {"'", false, false, true, {"'", "\\"}},
        {"\"", true, true, true, escapeSequences + QStringList{"\""}},
        {"<<<'", false, false, true, {}},
        {"<<<", true, true, true, escapeSequences}
    };
}

QRegularExpression PhpCode::identifier() const{
    static const QRegularExpression result("[A-Za-z_][A-Za-z0-9_]*");
    return result;
}

QRegularExpression PhpCode::trFunction() const{
    static const QRegularExpression result("^QObject\\s*::\\s*h?tr\\s*\\(");
    return result;
}
