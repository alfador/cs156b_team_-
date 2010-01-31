TEMPLATE = app
TARGET = 
DESTDIR = ../../bin
DEPENDPATH += .
INCLUDEPATH += .
QT = core
CONFIG -= app_bundle
CONFIG += release
#CONFIG += debug
# Input

include(../../src/netflixresultsgenerator.pri)

SOURCES += main.cpp
