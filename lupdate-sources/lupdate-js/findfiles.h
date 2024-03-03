#ifndef FINDFILES_H
#define FINDFILES_H

#include <QDir>
#include <QList>
#include "args.h"
#include "jscode.h"

QList<JsCode> findJsFilesInDir(const QDir &dir, const Options &options);

#endif // FINDFILES_H
