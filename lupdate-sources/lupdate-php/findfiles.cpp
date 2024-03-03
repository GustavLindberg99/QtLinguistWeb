#include <iostream>
#include "findfiles.h"

QList<PhpCode> findPhpFilesInDir(const QDir &dir, const Options &options){
    QList<PhpCode> result;
    for(const QFileInfo &fileInfo: dir.entryInfoList()){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".."){
            continue;
        }
        if(fileInfo.isDir()){
            if(options.recursive){
                result += findPhpFilesInDir(QDir(fileInfo.filePath()), options);
            }
        }
        else if(options.phpExtensions.contains(fileInfo.suffix())){
            QFile file(fileInfo.filePath());
            const std::string relativePath = fileInfo.filePath().toStdString();
            if(!file.open(QFile::ReadOnly)){
                continue;
            }
            PhpCode phpCode(file);
            if(phpCode.parse()){
                result.append(phpCode);
            }
            else if(!options.silent){
                std::cout << "Warning: failed to parse PHP file " << relativePath << ": "
                          << phpCode.lastError().toStdString() << std::endl;
            }
        }
    }
    return result;
}
