#ifndef JSORPHPCODE_H
#define JSORPHPCODE_H

#include <QRegularExpression>
#include <QStringList>
#include "code.h"
#include "tsfile.h"

class JsOrPhpCode : public Code{
public:
    using Code::Code;

    QList<TsFile::Translation> translations() const;
    QString lastError() const;
    bool parse() override;

protected:
    struct StringType{
        QString delimiter;
        bool supportsEscapes, supportsInterpolation, literalBackslashOnMissingEscapeSequence;
        QStringList escapeSequences;
    };

    virtual QList<QPair<QString, int>> codePortions() const = 0;
    virtual bool hasShellStyleComments() const = 0;
    virtual QList<StringType> stringTypes() const = 0;
    virtual QRegularExpression identifier() const = 0;
    virtual QRegularExpression trFunction() const = 0;

private:
    QList<TsFile::Translation> _translations;
    QString _lastError;
    int _lastErrorPos;
};

#endif // JSORPHPCODE_H
