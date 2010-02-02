QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
error("Use the qmake include with Qt4, on Debian that is qmake-qt4");
}

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
SOURCES += database.cpp \
	   movie.cpp \
           probe.cpp \
	   user.cpp \
           quickdatabase.cpp

HEADERS += database.h \
           movie.h \
           probe.h \
           rmse.h \
           user.h \
           quickdatabase.h

win32 {
    SOURCES += $$PWD/winmmap.cpp
    HEADERS += $$PWD/winmmap.h
}

