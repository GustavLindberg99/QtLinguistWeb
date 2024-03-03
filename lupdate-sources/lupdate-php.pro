QT += xml

CONFIG += console
CONFIG += c++17

DEFINES += LUPDATE_PHP

SOURCES += \
    common/args.cpp \
    common/code.cpp \
    common/jsorphpcode.cpp \
    common/optionscode.cpp \
    common/tsfile.cpp \
    lupdate-php/findfiles.cpp \
    lupdate-php/main.cpp \
    lupdate-php/phpcode.cpp

HEADERS += \
    common/args.h \
    common/code.h \
    common/jsorphpcode.h \
    common/optionscode.h \
    common/tsfile.h \
    lupdate-php/findfiles.h \
    lupdate-php/phpcode.h \
    lupdate-php/version.h

INCLUDEPATH += "$$PWD/lupdate-php" "$$PWD/common"

INCLUDEPATH += "$$PWD/../lexbor/include"
LIBS += -L"$$PWD/../lexbor/lib" -llexbor
