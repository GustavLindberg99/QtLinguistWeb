#include <QString>
#include <QStringList>
#include "args.h"
#include "jscode.h"
#include "findfiles.h"

int main(int argc, char **argv){
    //Parse the command line arguments
    const Options options = parseCommandLineArgs(argc, argv);
    if(options.success){
        return EXIT_SUCCESS;
    }
    else if(options.failure){
        return EXIT_FAILURE;
    }

    //Extract the strings from the JavaScript code
    const QList<JsCode> jsCodes = findJsFilesInDir(QDir("."), options);
    QList<TsFile::Translation> translations;
    for(const JsCode &jsCode: jsCodes){
        for(const TsFile::Translation &translation: jsCode.translations()){
            translations.append(translation);
        }
    }

    //Open the TS files
    updateTsFiles(options, translations);

    return EXIT_SUCCESS;
}
