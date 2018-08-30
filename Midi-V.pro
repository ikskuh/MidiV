#-------------------------------------------------
#
# Project created by QtCreator 2018-08-23T09:05:52
#
#-------------------------------------------------

TARGET = Midi-V
TEMPLATE = app
CONFIG -= qt
CONFIG += console

INCLUDEPATH += $$quote($$PWD/ext/gl3w/include)
INCLUDEPATH += $$quote($$PWD/ext/glm)
INCLUDEPATH += $$quote($$PWD/ext/stb)
INCLUDEPATH += $$quote($$PWD/ext/json)

DEPENDPATH += $$quote($$PWD/ext/gl3w/include)
DEPENDPATH += $$quote($$PWD/ext/glm)
DEPENDPATH += $$quote($$PWD/ext/stb)
DEPENDPATH += $$quote($$PWD/ext/json)

SOURCES += $$quote($$PWD/ext/gl3w/src/gl3w.c) \
    midiv.cpp \
    hal.cpp

CONFIG += c++17

*-g++: QMAKE_CXXFLAGS += -std=c++17

linux: {
    PACKAGES = drumstick-alsa sdl2
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PACKAGES)
    QMAKE_LFLAGS   += $$system(pkg-config --libs   $$PACKAGES)
    DEFINES += MIDIV_LINUX
}
win32: {
    DEFINES += MIDIV_WINDOWS
    LIBS += -lopengl32
    INCLUDEPATH += $$quote(C:\lib\SDL2-2.0.8\include)
    DEPENDPATH += $$quote(C:\lib\SDL2-2.0.8\include)
    LIBS += -L$$quote(C:\lib\SDL2-2.0.8\lib\x64) -lSDL2 -lSDL2main
}

SOURCES += \
        main.cpp \
    mshader.cpp \
    mvisualization.cpp \
    mmidistate.cpp \
    mresource.cpp \
    stbi_impl.c

HEADERS += \
    mshader.hpp \
    mvisualization.hpp \
    mmidistate.hpp \
    mresource.hpp \
    die.h \
    midiv.hpp \
    hal.hpp \
    debug.hpp \
    utils.hpp

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    visualization/plasmose.vis \
    visualization/plasmose.frag
