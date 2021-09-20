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
$ ./systray
```

## Screenshots

### First launch

![podman-systray screenshot](screenshot1.png)

### Not yet running

![podman-systray screenshot](screenshot2.png)

### Running machine

![podman-systray screenshot](screenshot3.png)

### SSH console

![podman-systray screenshot](screenshot4.png)

----

Added the podman logo to the [systray](https://doc.qt.io/qt-5/qtwidgets-desktop-systray-example.html) example, as a proof-of-concept.

See <https://github.com/heyvito/podman-macos> for the macOS version.
