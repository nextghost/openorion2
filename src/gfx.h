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

#define FONT_COLOR_MAX 58

#define RGB(x) (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define SRGB(x) 0xff, RGB(x)
#define RGBA(x, a) ((a) & 0xff), (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define TRANSPARENT 0, 0, 0, 0

class Image {
private:
	unsigned _width, _height, _frames, _frametime, _flags;
	unsigned *_textureIDs;
	uint8_t *_palette;

	// Do NOT implement
	Image(const Image &other);
	const Image &operator=(const Image &other);

protected:
	void decodeFrame(uint32_t *buffer, uint32_t *palette,
		MemoryReadStream &stream);
	void clear(void);

public:
	explicit Image(SeekableReadStream &stream,
		const uint8_t *base_palette = NULL);
	~Image(void);

	unsigned width(void) const;
	unsigned height(void) const;
	unsigned frameCount(void) const;
	unsigned frameTime(void) const;
	unsigned textureID(unsigned frame) const;
	const uint8_t *palette(void) const;

	void draw(int x, int y, unsigned frame = 0) const;
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
	int _textureIDs[FONT_COLOR_MAX], _shadowID, _outlineID;

	explicit Font(unsigned height);

	void createOutline(void);

public:
	~Font(void);

	unsigned height(void) const;
	unsigned charWidth(char ch) const;
	unsigned textWidth(const char *str) const;

	// Draw single character or entire string. X,Y are coordinates of
	// upper left corner of the text. Color is predefined font palette ID.
	// Both functions return X coordinate for drawing more text.
	int renderChar(int x, int y, unsigned color, char ch,
		unsigned outline = OUTLINE_NONE);
	int renderText(int x, int y, unsigned color, const char *str,
		unsigned outline = OUTLINE_NONE);
	int centerText(int x, int y, unsigned color, const char *str,
		unsigned outline = OUTLINE_NONE);

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
		unsigned height, ReadStream &stream);
	void loadFonts(SeekableReadStream &stream);
	void clear(void);

public:
	FontManager(unsigned lang_id);
	~FontManager(void);

	Font *getFont(unsigned id);
	unsigned fontCount(void) const;
};

#endif
