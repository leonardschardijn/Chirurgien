# Chirurgien

Chirurgien helps understand and manipulate file formats.

Features:

* File format highlighting
* Description panel interpreting the highlighted fields and offering additional information about the format
* Double click on a highlighted field for edition or extraction to a separate tab


## Dependencies

Chirurgien uses GTK4.

Chirurgien uses the Meson build system.


## Installation

### Chirurgien is available on Flathub

[<img width="240" src="https://flathub.org/assets/badges/flathub-badge-en.png">](https://www.flathub.org/apps/details/io.github.leonardschardijn.Chirurgien)


### Build Chirurgien

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
installed files in build/meson-logs/install-log.txt and remove them.


### Build Chirurgien as a flatpak

```
$ git clone https://github.com/leonardschardijn/Chirurgien.git
$ cd Chirurgien
$ flatpak-builder --repo=repo build io.github.leonardschardijn.Chirurgien.json
$ flatpak build-bundle repo chirurgien.flatpak io.github.leonardschardijn.Chirurgien
$ flatpak install --user chirurgien.flatpak
```

You could also use GNOME Builder.


## License

Chirurgien is released under the GNU General Public License (GPL) version 3
or later.

