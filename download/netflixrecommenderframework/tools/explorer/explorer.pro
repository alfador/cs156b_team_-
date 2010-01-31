TEMPLATE = app
TARGET = 
DESTDIR = ../../bin
DEPENDPATH += .
INCLUDEPATH += .
#CONFIG += debug
CONFIG -= app_bundle
CONFIG += release

# Input

include(../../src/netflixresultsgenerator.pri)

FORMS += mainwindow.ui
SOURCES += main.cpp
