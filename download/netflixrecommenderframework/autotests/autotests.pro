QT = core
CONFIG += qtestlib
#CONFIG += debug
CONFIG -= app_bundle
include(../src/netflixresultsgenerator.pri)

SOURCES		+= autotests.cpp
TARGET		= autotests
