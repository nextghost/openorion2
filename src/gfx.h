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

#define PALSIZE 1024

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

	unsigned _height, _glyphCount, _textureID;
	Glyph *_glyphs;

	explicit Font(unsigned height);

public:
	~Font(void);

	unsigned height(void) const;
	unsigned charWidth(char ch) const;
	unsigned textWidth(const char *str) const;

	void setPalette(const uint8_t *palette, unsigned colors);

	// Draw single character or entire string. X,Y are coordinates of
	// upper left corner of the text. Both functions return X coordinate
	// for drawing more text.
	int renderChar(int x, int y, char ch);
	int renderText(int x, int y, const char *str);

	friend class FontManager;
};

class FontManager {
private:
	Font **_fonts;
	unsigned _fontCount, _size;

	// Do NOT implement
	FontManager(const Font &other);
	const FontManager &operator=(const FontManager &other);

protected:
	void decodeGlyph(uint8_t *buf, unsigned width, unsigned pitch,
		unsigned height, ReadStream &stream);

public:
	FontManager(void);
	~FontManager(void);

	void loadFonts(SeekableReadStream &stream);
	void clear(void);
	Font *getFont(unsigned id);
	unsigned fontCount(void) const;
};

#endif
