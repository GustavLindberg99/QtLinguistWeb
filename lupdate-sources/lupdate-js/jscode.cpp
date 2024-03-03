#include "jscode.h"

QList<QPair<QString, int>> JsCode::codePortions() const{
    return {{this->contents(), 0}};
}

bool JsCode::hasShellStyleComments() const{
    return false;
}

QList<JsOrPhpCode::StringType> JsCode::stringTypes() const{
    //List of escape sequences in Javascript strings: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar#escape_sequences
    const QStringList escapeSequences{"0", "1", "2", "3", "4", "5", "6", "7", "'", "\"", "`", "\\", "n", "r", "v", "t", "b", "f", "\n", "$", "x", "u"};
    return {
        {"'", true, false, false, escapeSequences},
        {"\"", true, false, false, escapeSequences},
        {"`", true, true, false, escapeSequences}
    };
}

QRegularExpression JsCode::identifier() const{
    static const QRegularExpression result("[A-Za-z_$][A-Za-z0-9_$]*");
    return result;
}

QRegularExpression JsCode::trFunction() const{
    static const QRegularExpression result("^QObject\\s*\\.\\s*tr\\s*\\(");
    return result;
}
