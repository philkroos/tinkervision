tinkervision
============

Vision library for Tinkerforge RedBrick developed at FH-Bielefeld/Campus Minden.
http://www.tinkerforge.com/de/doc/Hardware/Bricks/RED_Brick.html

Tinkervision consists of
- a core component that manages camera input, conversion of
  frames between several supported formats, execution of external computer vision
  algorithms, and gives access to their results.
- Some basic example cv-algorithms provided as modules that are loadable by the core
  component.

The library is intended to be loaded as part of the Tinkerforge red-brick-apid,
though it can be used independently as well. However, the functionality and interface
was designed for the former usage.

# Requirements
The library is being developed and tested on Ubuntu 15.10, which comes with
g++-4.9.2. Earlier versions of g++ may fail, though 4.8 might be fine.
It is also tested to build and run on the target platform, the Tinkerforge Red Brick,
which operates a version of Linux Sunxi, also with g++-4.9.2

# Building and installing
The repository comes with a script to install required libraries and several
Makefiles to build the core component, the modules and tests.

1. Executing `python install_required.py` should install the missing libraries of the
   core component.
2. `make` or `DEBUG=1 make` builds the library.
3. `make install` installs necessary headers and the lib under `/usr/include`
   and `/usr/lib`.

## Building the modules
There is a top-level makefile in `src/modules` which builds all modules, with the
exception of the `streamer` module.  This requires additional external libraries,
which can be installed with a script found in this repository.
Each module can also be build and installed individually with the
corresponding makefile. To build and install all:

1. `cd src/modules`
2. `make` && `sudo make install`
3. `cd streamer`
4. `sudo python install_required.py`
5. `make && sudo make install`

# Usage
The library provides a C-interface, which can be used directly, as demonstrated in
the test files in `src/test/`. However, it is intended to be run as part
of the Tinkerforge `red-brick-apid` on the Tinkerforge `Red-Brick`.  Currently, the
supported image is 1.7.

## Usage on the Tinkerforge `Red-Brick`
To use the library, an extended version of the `red-brick-apid` is needed. On the
`Red Brick`:

1. `mkdir /home/tf/Software && cd /home/tf/Software`
2. `git clone https://github.com/philkroos/tinkervision`
3. `git clone https://github.com/philkroos/red-brick-apid`

Build and install `Tinkervision` and the provided modules, as described above. To
build the custom version of `red-brick-apid`:

1. `cd /home/tf/Software/red-brick-apid`
2. `git checkout tinkervision && cd src`
3. `WITH_VISION=yes python build_pkg.py`
4. `sudo dpkg -i redapid-2.0.2+vision_armhf.deb`

The next step is to test the library bindings. This repository comes with some
scripts which call into the generated Python-API. In general, all languages
supported by Tinkerforge should work, but only the Python bindings have been
tested yet for Tinkervision. In any case, it should work both directly on the
Red-Brick and on a PC which can access the device:

1. `mkdir /home/tf/TinkervisionTest`
2. `cd /home/tf/TinkervisionTest`
3. `git clone https://github.com/philkroos/tinkervision`
4. `git clone https://github.com/philkroos/generators`
5. `cd generators && git checkout tinkervision`
6. `mkdir /home/tf/Tinkervision/Test/tinkervision/src/test/red-brick/scripts/tinkerforge`
7. `cd python && WITH_TINKERVISION=1 python generate_python_bindings.py`
8. `cp ip_connection bindings/brick_red.py /home/tf/TinkervisionTest/tinkervision/src/test/red-brick/scripts/tinkerforge`
9. `cd /home/tf/TinkervisionTest/tinkervision/src/test/red-brick/scripts`
10. `touch tinkerforge/__init__.py`

Then you should be able to execute the scripts, but you need the UID of your
Red-Brick, which can be found e.g. with the Tinkerforge `brickv`. `main.py` and
`rb_setup.py` are utilities. The other files are tests which can be executed like:

- `./main.py <uid> colormotion`

If execution fails there may be another instance of ip_connection or brick_red.py in
the system, probably from an installation of brickv. The easiest solution would be to
remove all system-wide installed tinkerforge packages then, as well as the egg from
Pythons site-packages.

# Note
The library searches for loadable modules in two paths:
1. `/usr/lib/tinkervision` is fixed
2. The subdirectory `lib` in a custom path, which is configured at compile time:
   - `USER_PREFIX=/home/me/vision/ make`
   The default prefix is `/home/<current-user>/tv/`. The makefile tries to create
   a valid directory structure.

The provided modules will be installed to the system folder by default, but if `PRE`
is set during `make install`, they'll be installed to the user folder:
  `PRE=~/tv/lib make install` installs the module to the pre-configured library path.
