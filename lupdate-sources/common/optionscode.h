#ifndef OPTIONSCODE_H
#define OPTIONSCODE_H

#include <QString>
#include "code.h"

class OptionsCode : public Code{
public:
    using Code::Code;

    QStringList tsFiles() const;
    bool parse() override;

private:
    void parseOptionsString(QString options);

    QStringList _tsFiles;
};

#endif // OPTIONSCODE_H
