SOURCE_FILES = gfx.cpp gui.cpp lbx.cpp main.cpp sdl_screen.cpp stream.cpp
HEADER_FILES = gfx.h gui.h lbx.h screen.h stream.h

AM_CPPFLAGS = -DDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = openorion2
openorion2_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)
openorion2_LDADD = $(SDL2_MIXER_LIBS) $(SDL2_LIBS) -lSDL_mixer 
