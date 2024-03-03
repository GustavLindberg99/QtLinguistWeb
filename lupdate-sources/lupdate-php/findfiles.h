#ifndef FINDFILES_H
#define FINDFILES_H

#include <QDir>
#include <QList>
#include "args.h"
#include "phpcode.h"

QList<PhpCode> findPhpFilesInDir(const QDir &dir, const Options &options);

#endif // FINDFILES_H
