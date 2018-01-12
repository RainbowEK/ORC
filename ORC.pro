#-------------------------------------------------
#
# Project created by QtCreator 2017-07-07T19:57:24
#
#-------------------------------------------------

QT += core gui
QT += websockets
QT += winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "ORC_QT"

CONFIG   -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        ./src/main.cpp \
        ./src/wmain.cpp \
        ./src/wdebug.cpp \
        ./src/cfg_file.cpp

HEADERS += \
        ./src/globals.h \
        ./src/wmain.h \
        ./src/wdebug.h

FORMS += \
        ./src/wmain.ui \
        ./src/wdebug.ui

RESOURCES += \
        ./src/orc_resources.qrc


#win32 {
#    RC_FILE = orc.rc
#}

#RC_ICONS = orc-logo.ico
