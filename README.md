# OpenOrion2

**Project status:** partial savegame viewer, no gameplay (yet)

Open-source remake of classic 4X strategy game Master of Orion II: Battle at Antares.

## Dependencies

- LibSDL2
- SDL2\_mixer

## Installation

Clone this repository and build the sources:

    ./bootstrap
    ./configure
    make
    make install

Game data from the original Master of Orion II are required. Copy all \*.lbx files to the installation data directory (e.g. `/usr/local/share/openorion2/`) before running the compiled program.

Starting a new game is not implemented yet. You can copy saved games from the original game to OpenOrion2 user directory and load them from the main menu.
On Windows: `%APPDATA%\openorion2\`
On Linux: `~/.config/openorion2/`
