QT += xml

CONFIG += console
CONFIG += c++17

DEFINES += LUPDATE_JS

SOURCES += \
    common/args.cpp \
    common/code.cpp \
    common/jsorphpcode.cpp \
    common/optionscode.cpp \
    common/tsfile.cpp \
    lupdate-js/findfiles.cpp \
    lupdate-js/htmlcode.cpp \
    lupdate-js/jscode.cpp \
    lupdate-js/main.cpp

HEADERS += \
    common/args.h \
    common/code.h \
    common/jsorphpcode.h \
    common/languages.h \
    common/optionscode.h \
    common/tsfile.h \
    lupdate-js/findfiles.h \
    lupdate-js/htmlcode.h \
    lupdate-js/jscode.h \
    lupdate-js/version.h

INCLUDEPATH += "$$PWD/lupdate-js" "$$PWD/common"

INCLUDEPATH += "$$PWD/../lexbor/include"
LIBS += -L"$$PWD/../lexbor/lib" -llexbor
