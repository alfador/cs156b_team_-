TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG -= app_bundle
CONFIG += release
CONFIG += console
QT = core

include(../../src/netflixresultsgenerator.pri)

# Input
SOURCES += main.cpp
