SOURCE_FILES = gamestate.cpp gfx.cpp gui.cpp guimisc.cpp lbx.cpp main.cpp \
	mainmenu.cpp sdl_events.cpp sdl_screen.cpp sdl_utils.cpp stream.cpp \
	system.cpp utils.cpp
HEADER_FILES = gamestate.h gfx.h gui.h guimisc.h lbx.h mainmenu.h screen.h \
	stream.h system.h utils.h

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
