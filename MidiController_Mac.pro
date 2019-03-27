#-------------------------------------------------
#
# Project created by QtCreator 2017-08-20T09:10:49
#
#-------------------------------------------------

QT       += core gui multimedia xml multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MidiController
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS __MACOSX_CORE__

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES += resources.qrc

ICON = pixmap/midi.icns

LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation

SOURCES += \
		main.cpp \
		mainwindow.cpp \
		RtMidi.cpp \
		configappwindow.cpp \
		tmidi.cpp \
    configpadwindow.cpp \
    configplaylistwindow.cpp \
    configmidiwindow.cpp \
    tmediaplayer.cpp \
    scrolltext.cpp \
    tVideoWidget.cpp \
    dynamicfontsizelabel.cpp \
    dynamicfontsizepushbutton.cpp

HEADERS += \
		mainwindow.h \
		tPad.h \
		RtMidi.h \
		configappwindow.h \
		tmidi.h \
    configpadwindow.h \
    configplaylistwindow.h \
    tplaylist.h \
    common.h \
    configmidiwindow.h \
    tmediaplayer.h \
    scrolltext.h \
    tVideoWidget.h \
    dynamicfontsizelabel.h \
    dynamicfontsizepushbutton.h
