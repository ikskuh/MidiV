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
INCLUDEPATH += $$quote($$PWD/ext/rtmidi)

DEPENDPATH += $$quote($$PWD/ext/gl3w/include)
DEPENDPATH += $$quote($$PWD/ext/glm)
DEPENDPATH += $$quote($$PWD/ext/stb)
DEPENDPATH += $$quote($$PWD/ext/json)
DEPENDPATH += $$quote($$PWD/ext/rtmidi)

CONFIG += c++17

*-g++: QMAKE_CXXFLAGS += -std=c++17

debug: {
	DEFINES += MIDIV_DEBUG
}

linux: {
    DEFINES += MIDIV_LINUX __LINUX_ALSA__
    PACKAGES = sdl2 SDL2_ttf alsa
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PACKAGES)
    QMAKE_LFLAGS   += $$system(pkg-config --libs   $$PACKAGES)
	LIBS += -ldl
}
win32: {
    DEFINES += MIDIV_WINDOWS __WINDOWS_MM__
    LIBS += -lopengl32 -lwinmm
    INCLUDEPATH += $$quote(C:\lib\SDL2-2.0.8\include)
    DEPENDPATH += $$quote(C:\lib\SDL2-2.0.8\include)
    LIBS += -L$$quote(C:\lib\SDL2-2.0.8\lib\x64) -lSDL2 -lSDL2main
}
macos: {
    DEFINES += MIDIV_WINDOWS __MACOSX_CORE__
}

SOURCES += \
        main.cpp \
    mshader.cpp \
    mvisualization.cpp \
    mmidistate.cpp \
    mresource.cpp \
    stbi_impl.c \
    midiv.cpp \
    hal.cpp \
    mcctarget.cpp \
    resource-providers.cpp \
    providers/noiseprovider.cpp \
    providers/textprovider.cpp \
    providers/faketracker.cpp

HEADERS += \
    mshader.hpp \
    mvisualization.hpp \
    mmidistate.hpp \
    mresource.hpp \
    die.h \
    midiv.hpp \
    hal.hpp \
    debug.hpp \
    utils.hpp \
    mcctarget.hpp \
    ccs.hpp \
    providers/noiseprovider.hpp \
    providers/textprovider.hpp \
    providers/faketracker.hpp

SOURCES += $$quote($$PWD/ext/gl3w/src/gl3w.c)
SOURCES += $$quote($$PWD/ext/rtmidi/RtMidi.cpp)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    visualization/img/abstract1.jpg \
    visualization/img/pattern1.jpg \
    visualization/img/grid-sky.png \
    visualization/img/marbletile.png \
    visualization/img/plasma-gradient.png \
    visualization/img/plasma-pattern.png \
    visualization/img/retrowave_sun.png \
    visualization/img/rgba-noise.png \
    visualization/img/the-grid.png \
    visualization/image-rotate.vis \
    visualization/midi-pattern.vis \
    visualization/plasmose.vis \
    visualization/the-grid.vis \
    visualization/work.vis \
    visualization/base/flat-image.frag \
    visualization/base/midi-pattern.frag \
    visualization/base/plasmose.frag \
    visualization/base/sun.frag \
    visualization/base/the-grid.frag \
    visualization/pp/analog-glitch.frag \
    visualization/pp/grayscale.frag \
    visualization/pp/hueshift.frag \
    visualization/pp/lightscattering.frag \
    visualization/pp/pixelize.frag \
    visualization/pp/static-noise.frag \
    visualization/pp/vignette.frag \
    visualization/trafo/bulge.frag \
    visualization/trafo/rotate.frag \
    visualization/trafo/rotozoom.frag \
    visualization/overlay/sound-circle.frag \
    visualization/trafo/tunnel.frag \
    todo.txt \
    midiv.cfg \
    visualization/overlay/fire-01.frag \
    visualization/trafo/lens.frag \
    visualization/overlay/rasterbar.frag \
    visualization/overlay/scroller.frag \
    visualization/overlay/metaballs.frag \
    visualization/mixer/ghost.frag \
    visualization/overlay/image-overlay.frag
