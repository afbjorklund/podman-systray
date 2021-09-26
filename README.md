# Podman System Tray Icon

## Running

Open in [Qt Creator](https://doc.qt.io/qtcreator/) GUI:

```shell
qtcreator systray.pro
```

Or on the command line:

```console
$ qmake
$ make
...
$ ./podman-systray
```

## Screenshots

### First launch

![podman-systray screenshot](assets/screenshot1.png)

### Init parameters

![podman-systray screenshot](assets/screenshot2.png)


### Not yet running

![podman-systray screenshot](assets/screenshot3.png)

### Running machine

![podman-systray screenshot](assets/screenshot4.png)

### SSH console

![podman-systray screenshot](assets/screenshot5.png)

----

Added the podman logo to the [systray](https://doc.qt.io/qt-5/qtwidgets-desktop-systray-example.html) example, as a proof-of-concept.

See <https://github.com/heyvito/podman-macos> for the macOS version.

Unix builds (Linux, Mac) will need to install qtermwidget dependency:

See <https://github.com/lxqt/qtermwidget> (not available on Windows)
