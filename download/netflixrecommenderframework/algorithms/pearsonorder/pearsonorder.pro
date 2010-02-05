TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG -= app_bundle
#CONFIG += release
CONFIG += debug
CONFIG += console
QT = core

# Input

include(../../src/netflixresultsgenerator.pri)

SOURCES += main.cpp
