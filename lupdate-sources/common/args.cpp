#include <QFile>
#include <QFileInfo>
#include <iostream>
#include "args.h"
#include "languages.h"
#include "optionscode.h"
#include "version.h"

static void showHelp(){
    std::cout << "Usage:\n"
                 "    " PROGRAMNAME " [options] [html-or-js-files]...\n"
                 "\n"
              #ifdef LUPDATE_JS
                 "lupdate-js is part of Qt Linguist for Web, which is a port "
                 "of Qt Linguist's tool chain for JavaScript based websites. "
                 "It extracts translatable messages from JavaScript files, "
                 "as well as inline <script> tags in HTML and PHP files."
                 "Extracted messages are stored in textual translation source "
                 "files (typically Qt TS XML). New and modified messages can be merged "
                 "into existing TS files.\n"
              #else
                 "lupdate-php is part of Qt Linguist for Web, which is a port "
                 "of Qt Linguist's tool chain for PHP based websites. "
                 "It extracts translatable messages from PHP files."
                 "Extracted messages are stored in textual translation source "
                 "files (typically Qt TS XML). New and modified messages can be merged "
                 "into existing TS files.\n"
              #endif
                 "\n"
                 "Options:\n"
                 "    -help\n"
                 "        Display this information and exit.\n"
              #ifdef LUPDATE_JS
                 "    -html-extensions <ext>[,<ext>]...\n"
                 "        Process the files with the given extensions as HTML files, meaning that it searches for JavaScript code inside <script> tags.\n"
                 "        The extension list must be separated with commas, not with whitespace.\n"
                 "        Default: 'html,htm'.\n"
                 "    -js-extensions <ext>[,<ext>]...\n"
                 "        Process the files with the given extensions as JavaScript files.\n"
                 "        The extension list must be separated with commas, not with whitespace.\n"
                 "        Default: 'js'.\n"
              #endif
                 "    -list-languages\n"
                 "        Display a list of supported languages and exit.\n"
                 "    -no-obsolete\n"
                 "        Drop all obsolete and vanished strings.\n"
                 "    -no-recursive\n"
                 "        Do not recursively scan directories.\n"
                 "    -php-extensions <ext>[,<ext>]...\n"
                 "        Process the files with the given extensions as PHP files, meaning that it searches for JavaScript code inside <script> tags that are *not* within <?php ?>.\n"
                 "        The extension list must be separated with commas, not with whitespace.\n"
                 "        Default: 'php'.\n"
                 "    -pro <filename>\n"
                 "        Name of a file containing " LUPDATE_OPTIONS ".\n"
                 "        If not specified, searches for .htaccess, index.html, index.php in that order in the current working directory.\n"
                 "    -silent\n"
                 "        Do not explain what is being done.\n"
                 "    -ts <filename>...\n"
                 "        Specify the output file(s). This will override the TRANSLATIONS..\n"
                 "    -version\n"
                 "        Display the version of lupdate-js and exit."
              << std::endl;
}

static void showVersion(){
    std::cout << PROGRAMNAME << " version " PROGRAMVERSION
              << std::endl;
}

static void listLanguages(){
    for(const auto &language: languages){
        const std::string languageName = std::get<2>(language);
        std::cout << languageName << std::string(20 - languageName.length(), ' ') << std::get<0>(language) << std::endl;
    }
}

Options parseCommandLineArgs(int argc, char **argv){
    Options result;
    QString proFile;

    //Parse the arguments
    for(int i = 1; i < argc; i++){
        const QString arg = argv[i];
        QString value;
        if(arg == "-html-extensions" || arg == "-js-extensions" || arg == "-php-extensions" || arg == "-pro" || arg == "-ts"){
            i++;
            if(i >= argc){
                std::cerr << "Missing argument after " << arg.toStdString() << "." << std::endl;
                result.failure = true;
                return result;
            }
            value = argv[i];
        }
        if(arg == "-help" || arg == "-h" || arg == "--help"){
            showHelp();
            result.success = true;
            return result;
        }
        #ifdef LUPDATE_JS
            else if(arg == "-html-extensions"){
                const QStringList htmlExtensions = value.split(",");
                result.htmlExtensions = QSet(htmlExtensions.begin(), htmlExtensions.end());
            }
            else if(arg == "-js-extensions"){
                const QStringList jsExtensions = value.split(",");
                result.jsExtensions = QSet(jsExtensions.begin(), jsExtensions.end());
            }
        #endif
        else if(arg == "-list-languages"){
            listLanguages();
            result.success = true;
            return result;
        }
        else if(arg == "-no-obsolete"){
            result.noObsolete = true;
        }
        else if(arg == "-no-recursive"){
            result.recursive = false;
        }
        else if(arg == "-php-extensions"){
            const QStringList phpExtensions = value.split(",");
            result.phpExtensions = QSet(phpExtensions.begin(), phpExtensions.end());
        }
        else if(arg == "-pro"){
            proFile = value;
        }
        else if(arg == "-silent"){
            result.silent = true;
        }
        else if(arg == "-ts"){
            if(value[0] == '-' && value != "--"){
                std::cerr << "Missing file name after -ts. "
                             "To specify a file name that stars with a - character, specify -- before, "
                             "for example -ts -- -ts-file-starting-with-a-dash.ts." << std::endl;
                result.failure = true;
                return result;
            }
            for(; i < argc && value[0] != '-' && value != "--"; i++){
                value = argv[i];
                if(value != "--"){
                    result.tsFileNames.append(value);
                }
            }
        }
        else if(arg == "-version"){
            showVersion();
            result.success = true;
            return result;
        }
        else{
            std::cerr << "Unrecognized option '" << arg.toStdString() << "'. "
                         "To specify an HTML file name starting with a - character, specify the file with the -html option. "
                         "Use -help for a list of valid options." << std::endl;
            result.failure = true;
            return result;
        }
    }

    //Check that no extensions were specified for multiple file types
    #ifdef LUPDATE_JS
        const QSet<QString> overlap = QSet(result.htmlExtensions).intersect(result.jsExtensions)
                                      + QSet(result.htmlExtensions).intersect(result.phpExtensions)
                                      + QSet(result.jsExtensions).intersect(result.phpExtensions);
        if(!overlap.isEmpty()){
            std::cerr << "Extension " << overlap.begin()->toStdString() << " was specified in more than one of -html-extensions, -js-extensions and -php-extensions." << std::endl;
            result.failure = true;
            return result;
        }
    #endif

    //Find which TS files to create
    if(result.tsFileNames.isEmpty()){
        //If they weren't specified as a -ts argument, read the "pro file"
        if(proFile.isEmpty()){
            for(const char *fileName: {".htaccess", "index.html", "index.php"}){
                QFile file(fileName);
                if(!file.open(QFile::ReadOnly)){
                    continue;    //If the file can't be opened (most likely because it doesn't exist), just try the next one
                }
                OptionsCode optionsCode(file);
                if(!optionsCode.parse()){
                    continue;    //If this file doesn't contain any options, just try the next one
                }
                proFile = fileName;
                result.tsFileNames = optionsCode.tsFiles();
                break;    //If we found one file, we don't need to try the next ones
            }
        }
        else{
            QFile file(proFile);
            if(!file.open(QFile::ReadOnly)){
                std::cerr << "Could not open file " << proFile.toStdString() << " for reading." << std::endl;
                result.failure = true;
                return result;
            }
            OptionsCode optionsCode(file);
            if(!optionsCode.parse()){
                std::cerr << "File " << proFile.toStdString() << " does not contain any " LUPDATE_OPTIONS ".\n"
                             "Valid syntaxes for specifying " LUPDATE_OPTIONS " are:\n"
                             "    * Starting the file with `# " LUPDATE_OPTIONS "` followed by a newline, a new comment character (#) and the options (for .htaccess files)\n"
                             "    * Starting the file with `<!DOCTYPE html><!-- " LUPDATE_OPTIONS "` followed by a newline and the options (for HTML or PHP files)"
                          << std::endl;
                result.failure = true;
                return result;
            }
            result.tsFileNames = optionsCode.tsFiles();
        }
    }
    if(result.tsFileNames.isEmpty()){
        if(proFile.isEmpty()){
            std::cerr << "Error: no TS files specified. To fix this error, try one of the following:\n"
                         "    * Manually specify which translations files you would like to create using the -ts argument. For example, run this command:\n"
                         "        " PROGRAMNAME " -ts myproject_fr.ts\n"
                         "    * Add " LUPDATE_OPTIONS " to your .htaccess file. For example, add this to the beginning of your .htaccess file:\n"
                         "        # " LUPDATE_OPTIONS "\n"
                         "        # TRANSLATIONS = myproject_fr.ts\n"
                         "    * Add " LUPDATE_OPTIONS " to your index.html/index.php file. For example, add this to the beginning of your index file:\n"
                         "        <!DOCTYPE html>\n"
                         "        <!-- " LUPDATE_OPTIONS "\n"
                         "            TRANSLATIONS = myproject_fr.ts\n"
                         "        -->\n"
                         "    * Add " LUPDATE_OPTIONS " to a custom file using one of the two syntaxes above, then specify that file using the -pro argument. For example:\n"
                         "        " PROGRAMNAME " -pro file_where_you_added_" LUPDATE_OPTIONS ".html"
                      << std::endl;
        }
        else{
            std::cerr << "File " << proFile.toStdString() << " contains " LUPDATE_OPTIONS " but no valid TRANSLATIONS.\n";
            if(proFile == ".htaccess"){
                std::cerr << "You can add TRANSLATIONS to your .htaccess file by adding this to the beginning:\n"
                             "    # " LUPDATE_OPTIONS "\n"
                             "    # TRANSLATIONS = myproject_fr.ts"
                          << std::endl;
            }
            else{
                std::cerr << "You can add TRANSLATIONS to an HTML/PHP file by adding this to the beginning:\n"
                             "    <!DOCTYPE html>\n"
                             "    <!-- " LUPDATE_OPTIONS "\n"
                             "        TRANSLATIONS = myproject_fr.ts\n"
                             "    -->"
                          << std::endl;
            }
        }
        result.failure = true;
        return result;
    }

    return result;
}

void updateTsFiles(const Options &options, const QList<TsFile::Translation> &translations){
    for(const QString &tsFileName: options.tsFileNames){
        TsFile tsFile(tsFileName);
        tsFile.mkpath();
        if(!tsFile.open(QFile::ReadWrite)){
            std::cerr << "Failed to open file " << tsFileName.toStdString() << " for writing." << std::endl;
            continue;
        }
        if(!tsFile.loadTranslations()){
            std::cerr << tsFile.errorString().toStdString() << std::endl;
            continue;
        }
        const int numberOfNewTranslations = tsFile.setTranslations(translations, options);
        const int numberOfTranslations = tsFile.numberOfTranslations();
        const int numberOfObsoleteTranslations = tsFile.numberOfObsoleteTranslations();
        const int numberOfNonObsoleteTranslations = numberOfTranslations - numberOfObsoleteTranslations;
        if(!options.silent){
            std::cout << "Updating '" << tsFileName.toStdString() << "'...\n"
                                                                     "    Found " << numberOfNonObsoleteTranslations << " source text(s) (" << numberOfNewTranslations << " new and " << (numberOfNonObsoleteTranslations - numberOfNewTranslations) << " already existing)\n"
                                                                                                                                                                                            "    Kept " << numberOfObsoleteTranslations << " obsolete entries" << std::endl;
        }
        tsFile.writeTranslationsToFile();
    }
}
