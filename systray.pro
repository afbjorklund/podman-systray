HEADERS       = window.h
SOURCES       = main.cpp \
                window.cpp
RESOURCES     = systray.qrc

QT += widgets
requires(qtConfig(combobox))

TARGET        = podman-systray
unix {
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }
}
BINDIR        = $$PREFIX/bin

# install
target.path =$$BINDIR
INSTALLS += target

DISTFILES += \
    LICENSE

win32: DEFINES += QT_NO_TERMWIDGET

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += qtermwidget5
