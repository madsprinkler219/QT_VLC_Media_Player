#-------------------------------------------------
#
# Project created by QtCreator 2014-11-22T15:05:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settings.cpp

HEADERS  += mainwindow.h \
    settings.h

FORMS    += mainwindow.ui \
    settings.ui

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlc
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlcd
#else:unix: LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlc

#INCLUDEPATH += $$PWD/../Tools/lib_vlc/include
#DEPENDPATH += $$PWD/../Tools/lib_vlc/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlcd
else:unix: LIBS += -L$$PWD/../Tools/lib_vlc/lib/ -llibvlc

INCLUDEPATH += $$PWD/../Tools/lib_vlc/include
DEPENDPATH += $$PWD/../Tools/lib_vlc/include
