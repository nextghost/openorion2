/*
 * This file is part of OpenOrion2
 * Copyright (C) 2021 Martin Doucha
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GFX_H_
#define GFX_H_

#include "stream.h"
#include "utils.h"

#define PALSIZE 1024

#define FONTSIZE_TINY 0
#define FONTSIZE_SMALLER 1
#define FONTSIZE_SMALL 2
#define FONTSIZE_MEDIUM 3
#define FONTSIZE_BIG 4
#define FONTSIZE_TITLE 5
#define FONTSIZE_COUNT 6

#define OUTLINE_NONE 0
#define OUTLINE_SHADOW 1
#define OUTLINE_FULL 2

#define TITLE_COLOR_DEFAULT 0
#define TITLE_COLOR_HELP 1
#define TITLE_COLOR_MAX 2

#define FONT_COLOR_DEFAULT 0
#define FONT_COLOR_HELP 1
#define FONT_COLOR_ERROR 2
#define FONT_COLOR_SETTINGS 3
#define FONT_COLOR_SAVEGAME 4
#define FONT_COLOR_SAVEGAME_SEL 5
#define FONT_COLOR_STAR_NEUTRAL1 6
#define FONT_COLOR_STAR_NEUTRAL2 7
#define FONT_COLOR_STAR_NEUTRAL3 8
#define FONT_COLOR_STAR_NEUTRAL4 9
#define FONT_COLOR_STAR_NEUTRAL5 10

#define FONT_COLOR_PLAYER_RED1 11
#define FONT_COLOR_PLAYER_RED2 12
#define FONT_COLOR_PLAYER_RED3 13
#define FONT_COLOR_PLAYER_RED4 14
#define FONT_COLOR_PLAYER_RED5 15
#define FONT_COLOR_PLAYER_YELLOW1 16
#define FONT_COLOR_PLAYER_YELLOW2 17
#define FONT_COLOR_PLAYER_YELLOW3 18
#define FONT_COLOR_PLAYER_YELLOW4 19
#define FONT_COLOR_PLAYER_YELLOW5 20
#define FONT_COLOR_PLAYER_GREEN1 21
#define FONT_COLOR_PLAYER_GREEN2 22
#define FONT_COLOR_PLAYER_GREEN3 23
#define FONT_COLOR_PLAYER_GREEN4 24
#define FONT_COLOR_PLAYER_GREEN5 25
#define FONT_COLOR_PLAYER_SILVER1 26
#define FONT_COLOR_PLAYER_SILVER2 27
#define FONT_COLOR_PLAYER_SILVER3 28
#define FONT_COLOR_PLAYER_SILVER4 29
#define FONT_COLOR_PLAYER_SILVER5 30
#define FONT_COLOR_PLAYER_BLUE1 31
#define FONT_COLOR_PLAYER_BLUE2 32
#define FONT_COLOR_PLAYER_BLUE3 33
#define FONT_COLOR_PLAYER_BLUE4 34
#define FONT_COLOR_PLAYER_BLUE5 35
#define FONT_COLOR_PLAYER_BROWN1 36
#define FONT_COLOR_PLAYER_BROWN2 37
#define FONT_COLOR_PLAYER_BROWN3 38
#define FONT_COLOR_PLAYER_BROWN4 39
#define FONT_COLOR_PLAYER_BROWN5 40
#define FONT_COLOR_PLAYER_PURPLE1 41
#define FONT_COLOR_PLAYER_PURPLE2 42
#define FONT_COLOR_PLAYER_PURPLE3 43
#define FONT_COLOR_PLAYER_PURPLE4 44
#define FONT_COLOR_PLAYER_PURPLE5 45
#define FONT_COLOR_PLAYER_ORANGE1 46
#define FONT_COLOR_PLAYER_ORANGE2 47
#define FONT_COLOR_PLAYER_ORANGE3 48
#define FONT_COLOR_PLAYER_ORANGE4 49
#define FONT_COLOR_PLAYER_ORANGE5 50

#define FONT_COLOR_GALAXY_GUI 51
#define FONT_COLOR_DEFICIT1 52
#define FONT_COLOR_DEFICIT2 53
#define FONT_COLOR_DEFICIT3 54
#define FONT_COLOR_DEFICIT4 55
#define FONT_COLOR_DEFICIT5 56

#define FONT_COLOR_PLANET_LIST 57
#define FONT_COLOR_PLANET_LIST_BRIGHT 58
#define FONT_COLOR_PLANET_LIST_SILVER 59
#define FONT_COLOR_PLANET_LIST_SILVER_BRIGHT 60
#define FONT_COLOR_PLANET_LIST_BLUE 61
#define FONT_COLOR_PLANET_LIST_BLUE_BRIGHT 62
#define FONT_COLOR_PLANET_LIST_BROWN 63
#define FONT_COLOR_PLANET_LIST_BROWN_BRIGHT 64

#define FONT_COLOR_STAR_RED1 65
#define FONT_COLOR_STAR_RED2 66
#define FONT_COLOR_STAR_RED3 67
#define FONT_COLOR_STAR_RED4 68
#define FONT_COLOR_STAR_RED5 69
#define FONT_COLOR_STAR_YELLOW1 70
#define FONT_COLOR_STAR_YELLOW2 71
#define FONT_COLOR_STAR_YELLOW3 72
#define FONT_COLOR_STAR_YELLOW4 73
#define FONT_COLOR_STAR_YELLOW5 74
#define FONT_COLOR_STAR_GREEN1 75
#define FONT_COLOR_STAR_GREEN2 76
#define FONT_COLOR_STAR_GREEN3 77
#define FONT_COLOR_STAR_GREEN4 78
#define FONT_COLOR_STAR_GREEN5 79
#define FONT_COLOR_STAR_SILVER1 80
#define FONT_COLOR_STAR_SILVER2 81
#define FONT_COLOR_STAR_SILVER3 82
#define FONT_COLOR_STAR_SILVER4 83
#define FONT_COLOR_STAR_SILVER5 84
#define FONT_COLOR_STAR_BLUE1 85
#define FONT_COLOR_STAR_BLUE2 86
#define FONT_COLOR_STAR_BLUE3 87
#define FONT_COLOR_STAR_BLUE4 88
#define FONT_COLOR_STAR_BLUE5 89
#define FONT_COLOR_STAR_BROWN1 90
#define FONT_COLOR_STAR_BROWN2 91
#define FONT_COLOR_STAR_BROWN3 92
#define FONT_COLOR_STAR_BROWN4 93
#define FONT_COLOR_STAR_BROWN5 94
#define FONT_COLOR_STAR_PURPLE1 95
#define FONT_COLOR_STAR_PURPLE2 96
#define FONT_COLOR_STAR_PURPLE3 97
#define FONT_COLOR_STAR_PURPLE4 98
#define FONT_COLOR_STAR_PURPLE5 99
#define FONT_COLOR_STAR_ORANGE1 100
#define FONT_COLOR_STAR_ORANGE2 101
#define FONT_COLOR_STAR_ORANGE3 102
#define FONT_COLOR_STAR_ORANGE4 103
#define FONT_COLOR_STAR_ORANGE5 104

#define FONT_COLOR_FLEETLIST_FLEET_RED 105
#define FONT_COLOR_FLEETLIST_FLEET_YELLOW 106
#define FONT_COLOR_FLEETLIST_FLEET_GREEN 107
#define FONT_COLOR_FLEETLIST_FLEET_SILVER 108
#define FONT_COLOR_FLEETLIST_FLEET_BLUE 109
#define FONT_COLOR_FLEETLIST_FLEET_BROWN 110
#define FONT_COLOR_FLEETLIST_FLEET_PURPLE 111
#define FONT_COLOR_FLEETLIST_FLEET_ORANGE 112
#define FONT_COLOR_FLEETLIST_FLEET_MONSTER 113

#define FONT_COLOR_FLEETLIST_STAR_RED 114
#define FONT_COLOR_FLEETLIST_STAR_YELLOW 115
#define FONT_COLOR_FLEETLIST_STAR_GREEN 116
#define FONT_COLOR_FLEETLIST_STAR_SILVER 117
#define FONT_COLOR_FLEETLIST_STAR_BLUE 118
#define FONT_COLOR_FLEETLIST_STAR_BROWN 119
#define FONT_COLOR_FLEETLIST_STAR_PURPLE 120
#define FONT_COLOR_FLEETLIST_STAR_ORANGE 121
#define FONT_COLOR_FLEETLIST_STAR_NEUTRAL 122

#define FONT_COLOR_FLEETLIST_SHIPINFO_DESTINATION 123
#define FONT_COLOR_FLEETLIST_SPECDAMAGE 124

#define FONT_COLOR_COLONY_LIST 125
#define FONT_COLOR_COLONY_LIST_BRIGHT 126
#define FONT_COLOR_STAR_DESCRIPTION 127
#define FONT_COLOR_STARWIDGET_PLANET_INFO 128
#define FONT_COLOR_STARWIDGET_WORMHOLE 129

#define FONT_COLOR_LEADERLIST_NORMAL 130
#define FONT_COLOR_LEADERLIST_BRIGHT 131
#define FONT_COLOR_LEADERLIST_BRIGHT_NAME 132
#define FONT_COLOR_LEADERLIST_STAR_NEUTRAL 133
#define FONT_COLOR_LEADERLIST_STAR_SILVER 134

#define FONT_COLOR_RESEARCH_NORMAL 135
#define FONT_COLOR_RESEARCH_LIGHT 136
#define FONT_COLOR_RESEARCH_BRIGHT 137
#define FONT_COLOR_RESEARCH_BRIGHTER 138

#define FONT_COLOR_MAX 139

#define RGB(x) (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define SRGB(x) 0xff, RGB(x)
#define RGBA(x, a) ((a) & 0xff), (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define TRANSPARENT 0, 0, 0, 0

class Image {
private:
	unsigned _width, _height, _frames, _frametime, _flags, _palcount;
	unsigned *_textureIDs;
	uint8_t **_palettes;

	// Do NOT implement
	Image(const Image &other);
	const Image &operator=(const Image &other);

protected:
	void load(SeekableReadStream &stream, const uint8_t **base_palettes,
		unsigned palcount);
	void loadFrames(SeekableReadStream &stream, unsigned variant,
		const size_t *offsets);
	void decodeFrame(uint32_t *buffer, uint32_t *palette,
		MemoryReadStream &stream);
	void clear(void);

public:
	explicit Image(SeekableReadStream &stream,
		const uint8_t *base_palette = NULL);
	Image(SeekableReadStream &stream, const uint8_t **base_palettes,
		unsigned palcount);
	~Image(void);

	unsigned width(void) const;
	unsigned height(void) const;
	unsigned frameCount(void) const;
	unsigned frameTime(void) const;
	unsigned variantCount(void) const;
	unsigned textureID(unsigned frame) const;
	const uint8_t *palette(unsigned id = 0) const;

	void draw(int x, int y, unsigned frame = 0) const;
	void drawCentered(int x, int y, unsigned frame = 0) const;
};

class FontManager;

class Font {
private:
	// Do NOT implement
	Font(const Font &other);
	const Font &operator=(const Font &other);

protected:
	struct Glyph {
		unsigned offset, width;
	};

	unsigned _width, _height, _title, _glyphCount;
	Glyph *_glyphs;
	uint8_t *_bitmap;

	explicit Font(unsigned height);

	void setupPalette(uint8_t *palette, unsigned color, unsigned outline);
	int renderGlyph(int x, int y, const uint8_t *pal, char ch);

public:
	~Font(void);

	unsigned height(void) const;
	unsigned charWidth(char ch) const;
	unsigned textWidth(const char *str, unsigned charSpacing = 1) const;

	// Draw single character or entire string. X,Y are coordinates of
	// upper left corner of the text. Color is predefined font palette ID.
	// All font drawing methods return X coordinate for drawing more text
	// but renderChar() does not add any character spacing.
	int renderChar(int x, int y, unsigned color, char ch,
		unsigned outline = OUTLINE_NONE);
	int renderText(int x, int y, unsigned color, const char *str,
		unsigned outline = OUTLINE_NONE, unsigned charSpacing = 1);
	int centerText(int x, int y, unsigned color, const char *str,
		unsigned outline = OUTLINE_NONE, unsigned charSpacing = 1);
	int rightText(int x, int y, unsigned color, const char *str,
		unsigned outline = OUTLINE_NONE, unsigned charSpacing = 1);

	static const uint8_t *titlePalette(unsigned color);
	static const uint8_t *fontPalette(unsigned color);

	friend class FontManager;
};

class FontManager : public Recyclable {
private:
	Font *_fonts[FONTSIZE_COUNT];
	unsigned _fontCount;

	// Do NOT implement
	FontManager(const Font &other);
	const FontManager &operator=(const FontManager &other);

protected:
	void decodeGlyph(uint8_t *buf, unsigned width, unsigned pitch,
		unsigned height, unsigned colorCount, ReadStream &stream);
	void loadFonts(SeekableReadStream &stream);
	void clear(void);

public:
	FontManager(unsigned lang_id);
	~FontManager(void);

	Font *getFont(unsigned id);
	Font *fitFont(unsigned fontsize, unsigned maxwidth, const char *str);
	unsigned fontCount(void) const;
};

// Render text into limited space, reduce font size to fit
int fitText(int x, int y, unsigned maxwidth, unsigned fontsize, unsigned color,
	const char *str, unsigned outline = OUTLINE_NONE,
	unsigned charSpacing = 1);
int centerFitText(int x, int y, unsigned maxwidth, unsigned fontsize,
	unsigned color, const char *str, unsigned outline = OUTLINE_NONE,
	unsigned charSpacing = 1);

// Calculate frame for animation that loops from the last frame to the first
unsigned loopFrame(unsigned ticks, unsigned frametime, unsigned framecount);

// Calculate frame for animation that bounces back and forth between the first
// and last frame
unsigned bounceFrame(unsigned ticks, unsigned frametime, unsigned framecount);

// Set pixel to color only if the current pixel value is zero
void setBlankPixel(uint8_t *pixel, uint8_t color);

#endif
