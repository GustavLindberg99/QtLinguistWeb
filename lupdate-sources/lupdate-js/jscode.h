#ifndef JSCODE_H
#define JSCODE_H

#include "jsorphpcode.h"

class JsCode : public JsOrPhpCode{
public:
    using JsOrPhpCode::JsOrPhpCode;

protected:
    QList<QPair<QString, int>> codePortions() const override;
    bool hasShellStyleComments() const override;
    QList<StringType> stringTypes() const override;
    QRegularExpression identifier() const override;
    QRegularExpression trFunction() const override;
};

#endif // JSCODE_H
