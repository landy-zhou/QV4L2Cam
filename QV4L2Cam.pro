#-------------------------------------------------
#
# Project created by QtCreator 2015-11-19T10:42:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QV4L2Cam
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ImageProc.c \
    paintwidget.cpp \
    readthread.cpp

HEADERS  += mainwindow.h \
    ImageProc.h \
    paintwidget.h \
    readthread.h

FORMS    += mainwindow.ui
