# qtwin.pri
# 7/20/2011

DEFINES += WITH_WIN_QTWIN

INCLUDEPATH     += $$PWD
DEPENDPATH      += $$PWD

HEADERS += \
    $$PWD/qtwin.h \
    $$PWD/qtwinapi.h

SOURCES += \
    $$PWD/qtwin.cc

QT      += core gui
LIBS    += -lkernel32 -luser32 -lpsapi -lole32 #-lwinmm

# EOF
