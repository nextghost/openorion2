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

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "gfx.h"
#include "lbx.h"
#include "screen.h"

#define FLAG_JUNCTION	0x2000
#define FLAG_PALETTE	0x1000
#define FLAG_KEYCOLOR	0x0800
#define FLAG_FILLBG	0x0400
#define FLAG_NOCOMPRESS	0x0100

#define TITLE_PALSIZE 9
#define FONT_PALSIZE 4

#define RGB(x) 0xff, (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define RGBA(x, a) ((a) & 0xff), (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)
#define TRANSPARENT 0, 0, 0, 0

static const char *font_archives[LANG_COUNT] = {"fonts.lbx", "fontsg.lbx",
	"fontsf.lbx", "fontss.lbx", "fontsi.lbx"};

static uint8_t title_palettes[TITLE_COLOR_MAX][TITLE_PALSIZE * 4] = {
	{TRANSPARENT, RGB(0), RGB(0x302804), RGB(0x187814), RGB(0x187814),
		RGB(0x187814), RGB(0x209c1c), RGB(0x209c1c), RGB(0x209c1c)},
	{TRANSPARENT, RGB(0), RGB(0x043804), RGB(0x087008), RGB(0x087008),
		RGB(0x087008), RGB(0x489038), RGB(0x489038), RGB(0x489038)}
};

static uint8_t font_palettes[FONT_COLOR_MAX][FONT_PALSIZE * 4] = {
	{TRANSPARENT, RGB(0x302804), RGB(0x209c1c), RGB(0x187814)},
	{TRANSPARENT, RGB(0x043804), RGB(0x489038), RGB(0x087008)},
	{TRANSPARENT, RGB(0x580c08), RGB(0xcc1814), RGB(0x90141c)},
	{TRANSPARENT, RGB(0x3c1804), RGB(0xfc8800)},
	{TRANSPARENT, RGB(0x4c2800), RGB(0xc86408)},
	{TRANSPARENT, RGB(0x984c0c), RGB(0xfc8800)},

	// FONT_COLOR_STAR_NEUTRAL1
	{TRANSPARENT, RGB(0x000c00), RGB(0x606068)},
	{TRANSPARENT, RGB(0x000c00), RGB(0x6c6c74)},
	{TRANSPARENT, RGB(0x000c00), RGB(0x74747c)},
	{TRANSPARENT, RGB(0x000c00), RGB(0x84848c)},
	{TRANSPARENT, RGB(0x000c00), RGB(0x888890)},

	// FONT_COLOR_PLAYER_RED1
	{TRANSPARENT, RGB(0x6c0c0c), RGB(0x6c0c0c), RGB(0x6c0c0c)},
	{TRANSPARENT, RGB(0x6c0c0c), RGB(0x900808), RGB(0x6c0c0c)},
	{TRANSPARENT, RGB(0x6c0c0c), RGB(0xb40404), RGB(0x900808)},
	{TRANSPARENT, RGB(0x6c0c0c), RGB(0xd80000), RGB(0xb40404)},
	{TRANSPARENT, RGB(0x6c0c0c), RGB(0xfc0000), RGB(0xd80000)},

	// FONT_COLOR_PLAYER_YELLOW1
	{TRANSPARENT, RGB(0x20180c), RGB(0x443c0c), RGB(0x443c0c)},
	{TRANSPARENT, RGB(0x20180c), RGB(0x685c10), RGB(0x443c0c)},
	{TRANSPARENT, RGB(0x20180c), RGB(0x8c8010), RGB(0x685c10)},
	{TRANSPARENT, RGB(0x20180c), RGB(0xb0a014), RGB(0x8c8010)},
	{TRANSPARENT, RGB(0x20180c), RGB(0xd4c418), RGB(0xb0a014)},

	// FONT_COLOR_PLAYER_GREEN1
	{TRANSPARENT, RGB(0x001400), RGB(0x043004), RGB(0x043004)},
	{TRANSPARENT, RGB(0x001400), RGB(0x0c480c), RGB(0x043004)},
	{TRANSPARENT, RGB(0x001400), RGB(0x146410), RGB(0x0c480c)},
	{TRANSPARENT, RGB(0x001400), RGB(0x1c8014), RGB(0x146410)},
	{TRANSPARENT, RGB(0x001400), RGB(0x209c1c), RGB(0x1c8014)},

	// FONT_COLOR_PLAYER_SILVER1
	{TRANSPARENT, RGB(0x484848), RGB(0x686868), RGB(0x686868)},
	{TRANSPARENT, RGB(0x484848), RGB(0x848484), RGB(0x686868)},
	{TRANSPARENT, RGB(0x484848), RGB(0xa4a4a4), RGB(0x848484)},
	{TRANSPARENT, RGB(0x484848), RGB(0xc4c4c4), RGB(0xa4a4a4)},
	{TRANSPARENT, RGB(0x484848), RGB(0xe4e4e4), RGB(0xc4c4c4)},

	// FONT_COLOR_PLAYER_BLUE1
	{TRANSPARENT, RGB(0x081428), RGB(0x102844), RGB(0x102844)},
	{TRANSPARENT, RGB(0x081428), RGB(0x183c60), RGB(0x102844)},
	{TRANSPARENT, RGB(0x081428), RGB(0x205080), RGB(0x183c60)},
	{TRANSPARENT, RGB(0x081428), RGB(0x28689c), RGB(0x205080)},
	{TRANSPARENT, RGB(0x081428), RGB(0x347cb8), RGB(0x28689c)},

	// FONT_COLOR_PLAYER_BROWN1
	{TRANSPARENT, RGB(0x240808), RGB(0x3c2018), RGB(0x3c2018)},
	{TRANSPARENT, RGB(0x240808), RGB(0x543828), RGB(0x3c2018)},
	{TRANSPARENT, RGB(0x240808), RGB(0x70503c), RGB(0x543828)},
	{TRANSPARENT, RGB(0x240808), RGB(0x88684c), RGB(0x70503c)},
	{TRANSPARENT, RGB(0x240808), RGB(0xa48060), RGB(0x88684c)},

	// FONT_COLOR_PLAYER_PURPLE1
	{TRANSPARENT, RGB(0x2c1034), RGB(0x3c2048), RGB(0x3c2048)},
	{TRANSPARENT, RGB(0x2c1034), RGB(0x50305c), RGB(0x3c2048)},
	{TRANSPARENT, RGB(0x2c1034), RGB(0x644070), RGB(0x50305c)},
	{TRANSPARENT, RGB(0x2c1034), RGB(0x785084), RGB(0x644070)},
	{TRANSPARENT, RGB(0x2c1034), RGB(0x8c6098), RGB(0x785084)},

	// FONT_COLOR_PLAYER_ORANGE1
	{TRANSPARENT, RGB(0x3c1804), RGB(0x582804), RGB(0x582804)},
	{TRANSPARENT, RGB(0x3c1804), RGB(0x743804), RGB(0x582804)},
	{TRANSPARENT, RGB(0x3c1804), RGB(0x944808), RGB(0x743804)},
	{TRANSPARENT, RGB(0x3c1804), RGB(0xb05808), RGB(0x944808)},
	{TRANSPARENT, RGB(0x3c1804), RGB(0xd0680c), RGB(0xb05808)},

	// FONT_COLOR_GALAXY_GUI
	{TRANSPARENT, RGB(0x7c7c84), RGB(0xbcbcc4)},
	{TRANSPARENT, RGB(0x500c0c), RGB(0x741818)},
	{TRANSPARENT, RGB(0x500c0c), RGB(0x882020)},
	{TRANSPARENT, RGB(0x500c0c), RGB(0x9c302c)},
	{TRANSPARENT, RGB(0x500c0c), RGB(0xac4844)},
	{TRANSPARENT, RGB(0x500c0c), RGB(0xbc6058)},

	// FONT_COLOR_PLANET_LIST
	{TRANSPARENT, RGB(0x202726), RGB(0x69817c)},
};

Image::Image(SeekableReadStream &stream, const uint8_t *base_palette) :
	_width(0), _height(0), _frames(0), _textureIDs(NULL), _palette(NULL) {

	unsigned i, palstart, palsize;
	size_t *offsets;
	uint32_t *buffer;

	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	stream.readUint16LE();
	_frames = stream.readUint16LE();
	_frametime = stream.readUint16LE();
	_flags = stream.readUint16LE();

	if (!_width || !_height || !_frames) {
		throw std::runtime_error("Invalid image header");
	}

	if (!(_flags & FLAG_PALETTE) && !base_palette) {
		throw std::runtime_error("Palette missing");
	}

	offsets = new size_t[_frames + 1];

	for (i = 0; i <= _frames; i++) {
		offsets[i] = stream.readUint32LE();

		if (i && offsets[i] <= offsets[i-1]) {
			delete[] offsets;
			throw std::runtime_error("Invalid image frame offset");
		}
	}

	if (offsets[_frames] != (size_t)stream.size()) {
		delete[] offsets;
		throw std::runtime_error("Image data size mismatch");
	}

	_palette = new uint8_t[PALSIZE];

	if (base_palette) {
		memcpy(_palette, base_palette, PALSIZE);
	} else {
		memset(_palette, 0, PALSIZE);
	}

	if (_flags & FLAG_PALETTE) {
		palstart = stream.readUint16LE();
		palsize = stream.readUint16LE();

		if (palstart + palsize > 256) {
			delete[] offsets;
			delete[] _palette;
			throw std::runtime_error("Palette buffer overflow");
		}

		stream.read(_palette + 4 * palstart, palsize * 4);

		for (i = palstart; i < palstart + palsize; i++) {
			_palette[4*i] = 0xff;
			_palette[4*i+1] <<= 2;
			_palette[4*i+2] <<= 2;
			_palette[4*i+3] <<= 2;
		}
	}

	_textureIDs = new unsigned[_frames];
	buffer = new uint32_t[_width * _height];
	memset(buffer, 0, _width * _height * sizeof(uint32_t));

	for (i = 0; i < _frames; i++) {
		MemoryReadStream *substream = NULL;

		stream.seek(offsets[i], SEEK_SET);

		if (_flags & FLAG_FILLBG) {
			memset(buffer, 0, _width * _height * sizeof(uint32_t));
		}

		try {
			substream = stream.readStream(offsets[i+1]-offsets[i]);
			decodeFrame(buffer, (uint32_t*)_palette, *substream);
			_textureIDs[i] = registerTexture(_width, _height,
				buffer);
		} catch (...) {
			_frames = i;
			delete substream;
			delete[] offsets;
			delete[] buffer;
			clear();
			throw;
		}

		delete substream;
	}

	delete[] offsets;
	delete[] buffer;
}

Image::~Image(void) {
	clear();
}

void Image::clear(void) {
	unsigned i;

	for (i = 0; i < _frames; i++) {
		freeTexture(_textureIDs[i]);
	}

	delete[] _textureIDs;
	delete[] _palette;
}

void Image::decodeFrame(uint32_t *buffer, uint32_t *palette,
	MemoryReadStream &stream) {

	unsigned x, y, i, skip, size, tmp, keycolor = _flags & FLAG_KEYCOLOR;
	uint32_t *ptr = buffer;

	if (_flags & FLAG_NOCOMPRESS) {
		for (ptr = buffer, i = 0; i < _width * _height; ptr++, i++) {
			tmp = stream.readUint8();
			*ptr = tmp || !keycolor ? palette[tmp] : 0;
		}

		return;
	}

	size = stream.readUint16LE();
	y = stream.readUint16LE();

	if (size != 1) {
		throw std::runtime_error("First line marker != 1");
	}

	while (y < _height) {
		ptr = buffer + y * _width;

		for (x = 0; x < _width;) {
			size = stream.readUint16LE();
			skip = stream.readUint16LE();

			if (!size) {
				y += skip;
				break;
			}

			if (x + skip + size > _width) {
				throw std::runtime_error("Scan line overflow");
			}

			x += skip + size;
			ptr += skip;

			for (i = 0; i < size; i++, ptr++) {
				tmp = stream.readUint8();
				*ptr = tmp || !keycolor ? palette[tmp] : 0;
			}

			if (size % 2) {
				stream.readUint8();
			}

			if (stream.eos()) {
				throw std::runtime_error("Premature end of stream");
			}
		}
	}
}

unsigned Image::width(void) const {
	return _width;
}

unsigned Image::height(void) const {
	return _height;
}

unsigned Image::frameCount(void) const {
	return _frames;
}

unsigned Image::frameTime(void) const {
	return _frametime;
}

unsigned Image::textureID(unsigned frame) const {
	if (frame >= _frames) {
		throw std::out_of_range("Image frame ID out of range");
	}

	return _textureIDs[frame];
}

const uint8_t *Image::palette(void) const {
	return _palette;
}

void Image::draw(int x, int y, unsigned frame) const {
	drawTexture(textureID(frame), x, y);
}

Font::Font(unsigned height) : _width(0), _height(height), _title(0),
	_glyphCount(0), _glyphs(NULL), _bitmap(NULL), _shadowID(-1),
	_outlineID(-1) {

	size_t i;

	for (i = 0; i < FONT_COLOR_MAX; i++) {
		_textureIDs[i] = -1;
	}
}

Font::~Font(void) {
	size_t i;

	if (_outlineID >= 0) {
		freeTexture(_outlineID);
	}

	if (_shadowID >= 0) {
		freeTexture(_shadowID);
	}

	delete[] _glyphs;
	delete[] _bitmap;

	for (i = 0; i < FONT_COLOR_MAX; i++) {
		if (_textureIDs[i] >= 0) {
			freeTexture(_textureIDs[i]);
		}
	}
}

void Font::createOutline(void) {
	unsigned i, j, x, y, width, height;
	const uint8_t *src;
	uint8_t *buf, *dst;
	uint8_t shadowpal[3*4] = {TRANSPARENT, TRANSPARENT, RGB(0x101018)};
	uint8_t outlinepal[3*4] = {TRANSPARENT, RGB(0x101018), RGB(0x101018)};

	// Each glyph outline is 2 pixels wider and taller than the glyph
	width = _width + 2 * _glyphCount;
	height = _height + 2;
	buf = new uint8_t[width * height];
	memset(buf, 0, width * height * sizeof(uint8_t));

	for (i = 0; i < _glyphCount; i++) {
		src = _bitmap + _glyphs[i].offset;
		dst = buf + _glyphs[i].offset + 2 * i;

		for (y = 0; y < _height; y++) {
			for (x = 0; x < _glyphs[i].width; x++) {
				// Start from bottom right corner, otherwise
				// the shadow texture will miss a lot of pixels
				j = (_height - y - 1) * _width;
				j += _glyphs[i].width - x - 1;

				if (!src[j]) {
					continue;
				}

				// Outline for each non-blank pixel:
				// 111
				// 122
				// 122
				j = (_height - y - 1) * width;
				j += _glyphs[i].width - x - 1;
				dst[j] = dst[j+1] = dst[j+2] = 1;
				j += width;
				dst[j] = 1;
				dst[j+1] = dst[j+2] = 2;
				j += width;
				dst[j] = 1;
				dst[j+1] = dst[j+2] = 2;
			}
		}
	}

	try {
		_shadowID = registerTexture(width, height, buf, shadowpal, 0,
			3);
		_outlineID = registerTexture(width, height, buf, outlinepal, 0,
			3);
	} catch (...) {
		delete[] buf;
		throw;
	}

	delete[] buf;
}

unsigned Font::height(void) const {
	return _height;
}

unsigned Font::charWidth(char ch) const {
	unsigned idx = (unsigned char)ch;

	if (idx >= _glyphCount) {
		return 0;
	}

	return _glyphs[idx].width;
}

unsigned Font::textWidth(const char *str) const {
	unsigned idx, ret = 0;

	while ((idx = (unsigned char)*str++)) {
		if (idx >= _glyphCount) {
			continue;
		}

		ret += _glyphs[idx].width + 1;
	}

	return ret ? ret - 1 : 0;
}

int Font::renderChar(int x, int y, unsigned color, char ch, unsigned outline) {
	unsigned idx = (unsigned char)ch;

	if (idx >= _glyphCount) {
		return x;
	}

	if (color >= (_title ? TITLE_COLOR_MAX : FONT_COLOR_MAX)) {
		throw std::invalid_argument("Invalid font color");
	} else if (_textureIDs[color] < 0) {
		const uint8_t *pal;

		pal = _title ? title_palettes[color] : font_palettes[color];
		_textureIDs[color] = registerTexture(_width, _height, _bitmap,
			pal, 0, _title ? TITLE_PALSIZE : FONT_PALSIZE);
	}

	if (outline != OUTLINE_NONE) {
		unsigned tex;

		if (outline == OUTLINE_SHADOW) {
			tex = _shadowID;
		} else if (outline == OUTLINE_FULL) {
			tex = _outlineID;
		} else {
			throw std::runtime_error("Invalid outline type");
		}

		drawTextureTile(tex, x-1, y-1, _glyphs[idx].offset + 2*idx, 0,
			_glyphs[idx].width + 2, _height + 2);
	}

	drawTextureTile(_textureIDs[color], x, y, _glyphs[idx].offset, 0,
		_glyphs[idx].width, _height);
	return x + _glyphs[idx].width + 1;
}

int Font::renderText(int x, int y, unsigned color, const char *str,
	unsigned outline) {
	for (; *str; str++) {
		x = renderChar(x, y, color, *str, outline);
	}

	return x;
}

int Font::centerText(int x, int y, unsigned color, const char *str,
	unsigned outline) {
	return renderText(x - textWidth(str) / 2, y, color, str, outline);
}

FontManager::FontManager(unsigned lang_id) : _fontCount(0) {
	MemoryReadStream *stream;

	memset(_fonts, 0, FONTSIZE_COUNT * sizeof(Font*));
	stream = gameAssets->rawData(font_archives[lang_id], 0);

	try {
		loadFonts(*stream);
	} catch (...) {
		delete stream;
		throw;
	}

	delete stream;
}

FontManager::~FontManager(void) {
	clear();
}

void FontManager::decodeGlyph(uint8_t *buf, unsigned width, unsigned pitch,
	unsigned height, ReadStream &stream) {

	unsigned x, y;
	uint8_t tmp, *ptr;

	for (y = 0; y < height; y++) {
		x = 0;
		ptr = buf + y * pitch;

		while ((tmp = stream.readUint8()) != 0x80) {
			if (stream.eos()) {
				throw std::runtime_error("Premature end of font data");
			}

			if (tmp & 0x80) {
				ptr += tmp & 0x7f;
				x += tmp & 0x7f;
				continue;
			}

			if (x >= width) {
				throw std::runtime_error("Glyph line overflow");
			}

			*ptr++ = tmp;
			x++;
		}
	}
}

void FontManager::loadFonts(SeekableReadStream &stream) {
	unsigned i, j, x, size, glyphCount, fontCount = FONTSIZE_COUNT;
	unsigned offsets[256], magic[4] = {25, 50, 10, 0x404032};
	Font *ptr = NULL;
	Font::Glyph *glyphs = NULL, *gptr = NULL;
	uint8_t *bitmap = NULL;

	stream.seek(0, SEEK_SET);

	for (i = 0; i < 4; i++) {
		if (stream.readUint32LE() != magic[i]) {
			throw std::runtime_error("Invalid font header");
		}
	}

	// Load font sizes
	stream.seek(0x56c, SEEK_SET);

	for (i = 0; i < fontCount; i++) {
		size = stream.readUint16LE();

		if (stream.eos()) {
			throw std::runtime_error("Premature end of font data");
		}

		if (!size) {
			fontCount = i;
			break;
		}

		_fonts[i] = new Font(size);
	}

	// Load font data
	try {
		for (i = 0; i < fontCount; i++, _fontCount++) {
			glyphs = NULL;
			bitmap = NULL;
			ptr = _fonts[_fontCount];
			stream.seek(0xb9c + 1024 * i, SEEK_SET);
			offsets[0] = stream.readUint32LE();

			// Load glyph offsets
			for (j = 1; j < 256; j++) {
				offsets[j] = stream.readUint32LE();

				if (offsets[j] == offsets[j-1]) {
					break;
				} else if (offsets[j] < offsets[j-1]) {
					throw std::runtime_error("Invalid glyph offset");
				}
			}

			if (stream.eos()) {
				throw std::runtime_error("Premature end of font data");
			}

			// Load glyph metrics
			glyphCount = j - 1;
			glyphs = new Font::Glyph[glyphCount];
			stream.seek(0x59c + 256 * i, SEEK_SET);
			gptr = glyphs;

			for (x = 0, j = 0; j < glyphCount; j++, gptr++) {
				gptr->offset = x;
				gptr->width = stream.readUint8();
				x += gptr->width;
			}

			if (stream.eos()) {
				throw std::runtime_error("Premature end of font data");
			}

			// Create glyph bitmap
			// TODO: implement drawing glyphs with outline
			size = x;
			bitmap = new uint8_t[size * ptr->height()];
			memset(bitmap, 0, size * ptr->height());
			gptr = glyphs;

			for (j = 0; j < glyphCount; j++, gptr++) {
				stream.seek(0x239c + offsets[j], SEEK_SET);
				decodeGlyph(bitmap + gptr->offset,
					gptr->width, size, ptr->height(),
					stream);
			}

			ptr->_bitmap = bitmap;
			ptr->_width = size;
			ptr->_title = (i == FONTSIZE_TITLE);
			ptr->_glyphs = glyphs;
			ptr->_glyphCount = glyphCount;
			glyphs = NULL;
			bitmap = NULL;
			ptr->createOutline();
		}
	} catch (...) {
		delete[] glyphs;
		delete[] bitmap;
		clear();
		throw;
	}
}

void FontManager::clear(void) {
	size_t i;

	for (i = 0; i < _fontCount; i++) {
		delete _fonts[i];
	}

	_fontCount = 0;
}

Font *FontManager::getFont(unsigned id) {
	if (id >= _fontCount) {
		throw std::out_of_range("Invalid font ID");
	}

	return _fonts[id];
}

unsigned FontManager::fontCount(void) const {
	return _fontCount;
}
