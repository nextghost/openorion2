SOURCE_FILES = colony.cpp galaxy.cpp gamestate.cpp gfx.cpp gui.cpp \
	guimisc.cpp lbx.cpp main.cpp mainmenu.cpp officer.cpp screen.cpp \
	sdl_events.cpp sdl_screen.cpp sdl_utils.cpp ships.cpp stream.cpp \
	system.cpp tech.cpp utils.cpp
HEADER_FILES = colony.h galaxy.h gamestate.h gfx.h gui.h guimisc.h lang.h \
	lbx.h mainmenu.h officer.h screen.h ships.h stream.h system.h tech.h \
	utils.h

if SYSTEM_UNIX
SOURCE_FILES += unix.cpp
endif
if SYSTEM_WINDOWS
SOURCE_FILES += windows.cpp
endif

AM_CPPFLAGS = -DDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = openorion2
openorion2_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)
openorion2_LDADD = $(SDL2_LIBS)
