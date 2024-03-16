#ifndef ARGS_H
#define ARGS_H

#include <QSet>
#include <QString>
#include "tsfile.h"

struct Options final{
    #ifdef LUPDATE_JS
        QSet<QString> htmlExtensions{"html", "htm"};
        QSet<QString> jsExtensions{"js"};
    #endif
    QSet<QString> phpExtensions{"php"};
    QStringList tsFileNames;
    bool recursive = true;
    bool noObsolete = false;
    bool silent = false;
    bool success = false;    //True if it should exit immediately with EXIT_SUCCESS
    bool failure = false;    //True if it should exit immediately with EXIT_FAILURE
};

Options parseCommandLineArgs(int argc, char **argv);
void updateTsFiles(const Options &options, const QList<TsFile::Translation> &translations);

#endif // ARGS_H
