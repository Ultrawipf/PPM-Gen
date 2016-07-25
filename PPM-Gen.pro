#-------------------------------------------------
#
# Project created by QtCreator 2016-06-14T10:28:33
#
#-------------------------------------------------

QT       += core gui
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PPM-Gen
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ppmgenerator.cpp

HEADERS  += mainwindow.h \
    ppmgenerator.h

FORMS    += mainwindow.ui
