TARGET = qwinrt
CONFIG -= precompile_header

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QWinRTIntegrationPlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private

DEFINES += __WRL_NO_DEFAULT_LIB__

LIBS += $$QMAKE_LIBS_CORE

SOURCES = \
    main.cpp  \
    qwinrtbackingstore.cpp \
    qwinrtcursor.cpp \
    qwinrteventdispatcher.cpp \
    qwinrtinputcontext.cpp \
    qwinrtintegration.cpp \
    qwinrtscreen.cpp \
    qwinrtservices.cpp \
    qwinrtwindow.cpp

HEADERS = \
    qwinrtbackingstore.h \
    qwinrtcursor.h \
    qwinrteventdispatcher.h \
    qwinrtinputcontext.h \
    qwinrtintegration.h \
    qwinrtscreen.h \
    qwinrtservices.h \
    qwinrtwindow.h

contains(QT_CONFIG, opengles2) {
    DEFINES += Q_WINRT_GL
    SOURCES += qwinrteglcontext.cpp
    HEADERS += qwinrteglcontext.h
} else {
    SOURCES += qwinrtpageflipper.cpp
    HEADERS += qwinrtpageflipper.h
    LIBS += -ld3d11
}

OTHER_FILES += winrt.json

RESOURCES += \
    winrtfonts.qrc
