SOURCE_FILES = gfx.cpp gui.cpp lbx.cpp main.cpp mainmenu.cpp sdl_screen.cpp \
	stream.cpp savegame.cpp
HEADER_FILES = gfx.h gui.h lbx.h mainmenu.h screen.h stream.h savegame.h \
	system.h

if SYSTEM_UNIX
SOURCE_FILES += unix.cpp
endif
if SYSTEM_WINDOWS
SOURCE_FILES += windows.cpp
endif

AM_CPPFLAGS = -DDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = openorion2
openorion2_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)
openorion2_LDADD = $(SDL2_MIXER_LIBS) $(SDL2_LIBS) -lSDL_mixer 
