HEADERS       = window.h
SOURCES       = main.cpp \
                window.cpp
RESOURCES     = systray.qrc

QT += widgets
requires(qtConfig(combobox))

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/desktop/systray
INSTALLS += target

DISTFILES += \
    LICENSE

win32: DEFINES += QT_NO_TERMWIDGET

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += qtermwidget5
