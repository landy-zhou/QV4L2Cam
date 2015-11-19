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
    ImageProc.c

HEADERS  += mainwindow.h \
    ImageProc.h

FORMS    += mainwindow.ui
