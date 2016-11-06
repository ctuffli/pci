Instructions for building pci
=============================

When checked out from git, you will need to run `./autogen.sh` to setup
the autoconf files needed to build.

Run `./configure` to create the configuration needed for your
system. The full list of configure options is available by running
`configure --help`.

Run `make` to build the application. The executable will be `src/pci`.

Dependencies
============

pci uses [libpciaccess](https://cgit.freedesktop.org/xorg/lib/libpciaccess/)
and [libxo](https://github.com/Juniper/libxo)
libpciaccess is avaliable on most operating systems. Linux users may
find libxo packages [here](http://software.opensuse.org/download.html?project=home%3Actuffli&package=libxo)

