
QT       += core gui widgets

TARGET = ChainCopy
TEMPLATE = app

include ( p:\qt\BuildSystem\boost.pri )

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

LIBS += -lshell32

RESOURCES += \
    Resources.qrc
