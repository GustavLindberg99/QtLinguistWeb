#include <iostream>
#include "findfiles.h"
#include "htmlcode.h"

QList<JsCode> findJsFilesInDir(const QDir &dir, const Options &options){
    QList<JsCode> result;
    for(const QFileInfo &fileInfo: dir.entryInfoList()){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".."){
            continue;
        }
        if(fileInfo.isDir()){
            if(options.recursive){
                result += findJsFilesInDir(QDir(fileInfo.filePath()), options);
            }
        }
        else{
            const bool html = options.htmlExtensions.contains(fileInfo.suffix());
            const bool js = options.jsExtensions.contains(fileInfo.suffix());
            const bool php = options.phpExtensions.contains(fileInfo.suffix());

            QFile file(fileInfo.filePath());
            const std::string relativePath = fileInfo.filePath().toStdString();
            if(html || js || php){
                if(!file.open(QFile::ReadOnly)){
                    continue;
                }
            }

            if(js){
                JsCode jsCode(file);
                if(jsCode.parse()){
                    result.append(jsCode);
                }
                else if(!options.silent){
                    std::cout << "Warning: failed to parse JavaScript file " << relativePath << ": "
                              << jsCode.lastError().toStdString() << std::endl;
                }
            }
            else if(html || php){
                HtmlCode htmlCode = html ? HtmlCode(file) : HtmlCode::fromPhpFile(file);
                if(htmlCode.parse()){
                    for(JsCode jsCode: htmlCode.inlineJsScripts()){
                        if(jsCode.parse()){
                            result.append(jsCode);
                        }
                        else if(!options.silent){
                            std::cout << "Warning: failed to parse inline JavaScript code in HTML file " << relativePath << ": "
                                      << jsCode.lastError().toStdString() << std::endl;
                        }
                    }
                }
                else if(!options.silent){
                    std::cout << "Warning: failed to parse HTML file " << relativePath << "." << std::endl;
                }
            }
        }
    }
    return result;
}
