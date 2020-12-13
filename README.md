# Chirurgien

Chirurgien is a simple tool that helps to understand file formats.

## Dependencies

Chirurgien uses GTK3 and Amtk5.

Chirurgien uses the Meson build system.

## Installation

Build Chirurgien

```
$ git clone https://github.com/leonardschardijn/Chirurgien.git
$ cd Chirurgien
$ meson setup build
$ cd build
$ meson compile
$ meson install
```

Remember that Meson's default buildtype is debug.

If you wish to uninstall Chirurgien after installing this way look for the
installed files in build/meson-logs/install-log.txt and remove them. There
are no custom install scripts.


Build Chirurgien as a flatpak

```
$ git clone https://github.com/leonardschardijn/Chirurgien.git
$ cd Chirurgien
$ flatpak-builder --repo=repo build io.github.leonardschardijn.Chirurgien.json
$ flatpak build-bundle repo chirurgien.flatpak io.github.leonardschardijn.Chirurgien
$ flatpak install --user chirurgien.flatpak
```

You could also use GNOME Builder.

# License

Chirurgien is released under the GNU General Public License (GPL) version 3
or later.

