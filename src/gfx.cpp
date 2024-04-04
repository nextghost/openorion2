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

static const char *font_archives[LANG_COUNT] = {"fonts.lbx", "fontsg.lbx",
	"fontsf.lbx", "fontss.lbx", "fontsi.lbx"};

static const uint8_t title_palettes[TITLE_COLOR_MAX][TITLE_PALSIZE * 4] = {
	{TRANSPARENT, SRGB(0), SRGB(0x302804), SRGB(0x187814), SRGB(0x187814),
		SRGB(0x187814), SRGB(0x209c1c), SRGB(0x209c1c), SRGB(0x209c1c)},
	{TRANSPARENT, SRGB(0), SRGB(0x043804), SRGB(0x087008), SRGB(0x087008),
		SRGB(0x087008), SRGB(0x489038), SRGB(0x489038), SRGB(0x489038)}
};

static const uint8_t font_palettes[FONT_COLOR_MAX][FONT_PALSIZE * 4] = {
	{TRANSPARENT, SRGB(0x302804), SRGB(0x209c1c), SRGB(0x187814)},
	{TRANSPARENT, SRGB(0x043804), SRGB(0x489038), SRGB(0x087008)},
	{TRANSPARENT, SRGB(0x580c08), SRGB(0xcc1814), SRGB(0x90141c)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0xfc8800)},
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xc86408)},
	{TRANSPARENT, SRGB(0x984c0c), SRGB(0xfc8800)},

	// FONT_COLOR_STAR_NEUTRAL1
	{TRANSPARENT, SRGB(0x000c00), SRGB(0x606068)},
	{TRANSPARENT, SRGB(0x000c00), SRGB(0x6c6c74)},
	{TRANSPARENT, SRGB(0x000c00), SRGB(0x74747c)},
	{TRANSPARENT, SRGB(0x000c00), SRGB(0x84848c)},
	{TRANSPARENT, SRGB(0x000c00), SRGB(0x888890)},

	// FONT_COLOR_PLAYER_RED1
	{TRANSPARENT, SRGB(0x6c0c0c), SRGB(0x6c0c0c), SRGB(0x6c0c0c)},
	{TRANSPARENT, SRGB(0x6c0c0c), SRGB(0x900808), SRGB(0x6c0c0c)},
	{TRANSPARENT, SRGB(0x6c0c0c), SRGB(0xb40404), SRGB(0x900808)},
	{TRANSPARENT, SRGB(0x6c0c0c), SRGB(0xd80000), SRGB(0xb40404)},
	{TRANSPARENT, SRGB(0x6c0c0c), SRGB(0xfc0000), SRGB(0xd80000)},

	// FONT_COLOR_PLAYER_YELLOW1
	{TRANSPARENT, SRGB(0x20180c), SRGB(0x443c0c), SRGB(0x443c0c)},
	{TRANSPARENT, SRGB(0x20180c), SRGB(0x685c10), SRGB(0x443c0c)},
	{TRANSPARENT, SRGB(0x20180c), SRGB(0x8c8010), SRGB(0x685c10)},
	{TRANSPARENT, SRGB(0x20180c), SRGB(0xb0a014), SRGB(0x8c8010)},
	{TRANSPARENT, SRGB(0x20180c), SRGB(0xd4c418), SRGB(0xb0a014)},

	// FONT_COLOR_PLAYER_GREEN1
	{TRANSPARENT, SRGB(0x001400), SRGB(0x043004), SRGB(0x043004)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x0c480c), SRGB(0x043004)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x146410), SRGB(0x0c480c)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x1c8014), SRGB(0x146410)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x209c1c), SRGB(0x1c8014)},

	// FONT_COLOR_PLAYER_SILVER1
	{TRANSPARENT, SRGB(0x484848), SRGB(0x686868), SRGB(0x686868)},
	{TRANSPARENT, SRGB(0x484848), SRGB(0x848484), SRGB(0x686868)},
	{TRANSPARENT, SRGB(0x484848), SRGB(0xa4a4a4), SRGB(0x848484)},
	{TRANSPARENT, SRGB(0x484848), SRGB(0xc4c4c4), SRGB(0xa4a4a4)},
	{TRANSPARENT, SRGB(0x484848), SRGB(0xe4e4e4), SRGB(0xc4c4c4)},

	// FONT_COLOR_PLAYER_BLUE1
	{TRANSPARENT, SRGB(0x081428), SRGB(0x102844), SRGB(0x102844)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x183c60), SRGB(0x102844)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x205080), SRGB(0x183c60)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x28689c), SRGB(0x205080)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x347cb8), SRGB(0x28689c)},

	// FONT_COLOR_PLAYER_BROWN1
	{TRANSPARENT, SRGB(0x240808), SRGB(0x3c2018), SRGB(0x3c2018)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0x543828), SRGB(0x3c2018)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0x70503c), SRGB(0x543828)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0x88684c), SRGB(0x70503c)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0xa48060), SRGB(0x88684c)},

	// FONT_COLOR_PLAYER_PURPLE1
	{TRANSPARENT, SRGB(0x2c1034), SRGB(0x3c2048), SRGB(0x3c2048)},
	{TRANSPARENT, SRGB(0x2c1034), SRGB(0x50305c), SRGB(0x3c2048)},
	{TRANSPARENT, SRGB(0x2c1034), SRGB(0x644070), SRGB(0x50305c)},
	{TRANSPARENT, SRGB(0x2c1034), SRGB(0x785084), SRGB(0x644070)},
	{TRANSPARENT, SRGB(0x2c1034), SRGB(0x8c6098), SRGB(0x785084)},

	// FONT_COLOR_PLAYER_ORANGE1
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0x582804), SRGB(0x582804)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0x743804), SRGB(0x582804)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0x944808), SRGB(0x743804)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0xb05808), SRGB(0x944808)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0xd0680c), SRGB(0xb05808)},

	// FONT_COLOR_GALAXY_GUI
	{TRANSPARENT, SRGB(0x7c7c84), SRGB(0xbcbcc4)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0x741818)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0x882020)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0x9c302c)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0xac4844)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0xbc6058)},

	// FONT_COLOR_PLANET_LIST
	{TRANSPARENT, SRGB(0x202828), SRGB(0x68807c)},
	{TRANSPARENT, SRGB(0x202828), SRGB(0x80b0b8)},

	// FONT_COLOR_PLANET_LIST_SILVER
	{TRANSPARENT, SRGB(0x242828), SRGB(0xb8b8b8)},
	{TRANSPARENT, SRGB(0x242828), SRGB(0xc8c8c8)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x284c88)},
	{TRANSPARENT, SRGB(0x081428), SRGB(0x305ca0)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0x885840)},
	{TRANSPARENT, SRGB(0x240808), SRGB(0xa47050)},

	// FONT_COLOR_STAR_RED
	{TRANSPARENT, SRGB(0x400808), SRGB(0x882020)},
	{TRANSPARENT, SRGB(0x400808), SRGB(0x9c302c)},
	{TRANSPARENT, SRGB(0x400808), SRGB(0xac4844)},
	{TRANSPARENT, SRGB(0x400808), SRGB(0xc8706c)},
	{TRANSPARENT, SRGB(0x400808), SRGB(0xd8847c)},

	// FONT_COLOR_STAR_YELLOW
	{TRANSPARENT, SRGB(0x4c3800), SRGB(0x907808)},
	{TRANSPARENT, SRGB(0x4c3800), SRGB(0xa89014)},
	{TRANSPARENT, SRGB(0x4c3800), SRGB(0xc0a81c)},
	{TRANSPARENT, SRGB(0x4c3800), SRGB(0xe8d070)},
	{TRANSPARENT, SRGB(0x4c3800), SRGB(0xf4e094)},

	// FONT_COLOR_STAR_GREEN
	{TRANSPARENT, SRGB(0x043804), SRGB(0x2c8024)},
	{TRANSPARENT, SRGB(0x043804), SRGB(0x489038)},
	{TRANSPARENT, SRGB(0x043804), SRGB(0x64a84c)},
	{TRANSPARENT, SRGB(0x043804), SRGB(0xb8e488)},
	{TRANSPARENT, SRGB(0x043804), SRGB(0x00fc00)},

	// FONT_COLOR_STAR_SILVER
	{TRANSPARENT, SRGB(0x6c6c74), SRGB(0xacacb4)},
	{TRANSPARENT, SRGB(0x6c6c74), SRGB(0xb4b4bc)},
	{TRANSPARENT, SRGB(0x6c6c74), SRGB(0xbcbcc4)},
	{TRANSPARENT, SRGB(0x6c6c74), SRGB(0xccccd4)},
	{TRANSPARENT, SRGB(0x6c6c74), SRGB(0xdcdce4)},

	// FONT_COLOR_STAR_BLUE
	{TRANSPARENT, SRGB(0x384c6c), SRGB(0x5c7ca0)},
	{TRANSPARENT, SRGB(0x384c6c), SRGB(0x688cb0)},
	{TRANSPARENT, SRGB(0x384c6c), SRGB(0x789cc0)},
	{TRANSPARENT, SRGB(0x384c6c), SRGB(0x94c0dc)},
	{TRANSPARENT, SRGB(0x384c6c), SRGB(0xa0d0ec)},

	// FONT_COLOR_STAR_BROWN
	{TRANSPARENT, SRGB(0x3c1410), SRGB(0x8c5840)},
	{TRANSPARENT, SRGB(0x3c1410), SRGB(0x9c6448)},
	{TRANSPARENT, SRGB(0x3c1410), SRGB(0xa87450)},
	{TRANSPARENT, SRGB(0x3c1410), SRGB(0xd09c70)},
	{TRANSPARENT, SRGB(0x3c1410), SRGB(0xdcb480)},

	// FONT_COLOR_STAR_PURPLE
	{TRANSPARENT, SRGB(0x2c1018), SRGB(0x703c5c)},
	{TRANSPARENT, SRGB(0x2c1018), SRGB(0x7c4868)},
	{TRANSPARENT, SRGB(0x2c1018), SRGB(0x8c547c)},
	{TRANSPARENT, SRGB(0x2c1018), SRGB(0xa46c9c)},
	{TRANSPARENT, SRGB(0x2c1018), SRGB(0xb478ac)},

	// FONT_COLOR_STAR_ORANGE
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xb0580c)},
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xc86408)},
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xe07c20)},
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xec9c44)},
	{TRANSPARENT, SRGB(0x4c2800), SRGB(0xecb45c)},

	// FONT_COLOR_FLEETLIST_FLEET_RED
	{TRANSPARENT, SRGB(0x300404), SRGB(0xac4844)},
	{TRANSPARENT, SRGB(0x3c2c00), SRGB(0xc0a81c)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x64a84c)},
	{TRANSPARENT, SRGB(0x000c10), SRGB(0x687484)},
	{TRANSPARENT, SRGB(0x283458), SRGB(0x88b0d0)},
	{TRANSPARENT, SRGB(0x300c0c), SRGB(0xa87450)},
	{TRANSPARENT, SRGB(0x200c10), SRGB(0x8c547c)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0xe07c20)},

	// FONT_COLOR_FLEETLIST_FLEET_MONSTER
	{TRANSPARENT, SRGB(0x641010), SRGB(0x9c302c), SRGB(0x882020)},

	// FONT_COLOR_FLEETLIST_STAR_RED
	{TRANSPARENT, SRGB(0x300404), SRGB(0x9c302c)},
	{TRANSPARENT, SRGB(0x3c2c00), SRGB(0xa89014)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x489038)},
	{TRANSPARENT, SRGB(0x84848c), SRGB(0xacacb4)},
	{TRANSPARENT, SRGB(0x283458), SRGB(0x789cc0)},
	{TRANSPARENT, SRGB(0x300c0c), SRGB(0x9c6448)},
	{TRANSPARENT, SRGB(0x200c10), SRGB(0x7c4868)},
	{TRANSPARENT, SRGB(0x3c1804), SRGB(0xc86408), SRGB(0x7c4000)},

	// FONT_COLOR_FLEETLIST_STAR_NEUTRAL
	{TRANSPARENT, SRGB(0x242428), SRGB(0x888890)},
	{TRANSPARENT, SRGB(0x087008), SRGB(0xb8e488)},
	{TRANSPARENT, SRGB(0x500c0c), SRGB(0xc40000)},

	// FONT_COLOR_COLONY_LIST
	{TRANSPARENT, SRGB(0x141420), SRGB(0x6c688c)},
	{TRANSPARENT, SRGB(0x080814), SRGB(0x80a0bc)},
	{TRANSPARENT, SRGB(0x181c40), SRGB(0x506c90)},
	{TRANSPARENT, SRGB(0x181c40), SRGB(0x688cb0)},
	{TRANSPARENT, SRGB(0x001400), SRGB(0x2c8024)},

	// FONT_COLOR_LEADER_LIST_NORMAL
	{TRANSPARENT, SRGB(0x20284c), SRGB(0x789cc0)},
	{TRANSPARENT, SRGB(0x20284c), SRGB(0xa0d0ec)},
	{TRANSPARENT, SRGB(0x283458), SRGB(0xa0d0ec)},
	{TRANSPARENT, SRGB(0x00141c), SRGB(0x586478)},
	{TRANSPARENT, SRGB(0x484850), SRGB(0xb4b4bc)},

	// FONT_COLOR_RESEARCH_NORMAL
	{TRANSPARENT, SRGB(0x181818), SRGB(0x047800), SRGB(0x047800)},
	{TRANSPARENT, SRGB(0x181818), SRGB(0x28c800), SRGB(0x28c800)},
	{TRANSPARENT, SRGB(0x181818), SRGB(0x64d000), SRGB(0x64d000)},
};

Image::Image(SeekableReadStream &stream, const uint8_t *base_palette) :
	_width(0), _height(0), _frames(0), _palcount(0), _textureIDs(NULL),
	_palettes(NULL) {

	load(stream, &base_palette, base_palette ? 1 : 0);
}

Image::Image(SeekableReadStream &stream, const uint8_t **base_palettes,
	unsigned palcount) : _width(0), _height(0), _frames(0), _palcount(0),
	_textureIDs(NULL), _palettes(NULL) {

	load(stream, base_palettes, palcount);
}

Image::~Image(void) {
	clear();
}

void Image::load(SeekableReadStream &stream, const uint8_t **base_palettes,
	unsigned palcount) {

	unsigned i, palstart, palsize, framecount;
	size_t *offsets;

	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	stream.readUint16LE();
	framecount = stream.readUint16LE();
	_frametime = stream.readUint16LE();
	_flags = stream.readUint16LE();

	if (!_width || !_height || !framecount) {
		throw std::runtime_error("Invalid image header");
	}

	if (!(_flags & FLAG_PALETTE) && !palcount) {
		throw std::runtime_error("Palette missing");
	}

	offsets = new size_t[framecount + 1];

	for (i = 0; i <= framecount; i++) {
		offsets[i] = stream.readUint32LE();

		if (i && offsets[i] <= offsets[i-1]) {
			delete[] offsets;
			throw std::runtime_error("Invalid image frame offset");
		}
	}

	if (offsets[framecount] != (size_t)stream.size()) {
		delete[] offsets;
		throw std::runtime_error("Image data size mismatch");
	}

	_palcount = palcount ? palcount : 1;

	try {
		_palettes = new uint8_t*[_palcount];
	} catch (...) {
		delete[] offsets;
		throw;
	}

	memset(_palettes, 0, _palcount * sizeof(uint8_t*));

	for (i = 0; i < _palcount; i++) {
		try {
			_palettes[i] = new uint8_t[PALSIZE];
		} catch (...) {
			delete[] offsets;
			clear();
			throw;
		}

		if (i < palcount && base_palettes[i]) {
			memcpy(_palettes[i], base_palettes[i], PALSIZE);
		} else if (!(_flags & FLAG_PALETTE)) {
			delete[] offsets;
			clear();
			throw std::runtime_error("Palette missing");
		} else {
			memset(_palettes[i], 0, PALSIZE);
		}
	}

	if (_flags & FLAG_PALETTE) {
		palstart = stream.readUint16LE();
		palsize = stream.readUint16LE();

		if (palstart + palsize > 256) {
			delete[] offsets;
			clear();
			throw std::runtime_error("Palette buffer overflow");
		}

		stream.read(_palettes[0] + 4 * palstart, palsize * 4);

		for (i = palstart; i < palstart + palsize; i++) {
			_palettes[0][4*i] = 0xff;
			_palettes[0][4*i+1] <<= 2;
			_palettes[0][4*i+2] <<= 2;
			_palettes[0][4*i+3] <<= 2;
		}

		for (i = 1; i < _palcount; i++) {
			memcpy(_palettes[i] + 4 * palstart,
				_palettes[0] + 4 * palstart, 4 * palsize);
		}
	}

	try {
		_textureIDs = new unsigned[framecount * _palcount];
	} catch (...) {
		delete[] offsets;
		clear();
		throw;
	}

	_frames = framecount;

	for (i = 0; i < _palcount; i++) {
		try {
			loadFrames(stream, i, offsets);
		} catch (...) {
			delete[] offsets;
			clear();
			throw;
		}
	}

	delete[] offsets;
}

void Image::loadFrames(SeekableReadStream &stream, unsigned variant,
	const size_t *offsets) {

	unsigned i, fpos = variant * _frames;
	uint32_t *buffer;

	buffer = new uint32_t[_width * _height];
	memset(buffer, 0, _width * _height * sizeof(uint32_t));

	for (i = 0; i < _frames; i++, fpos++) {
		MemoryReadStream *substream = NULL;

		stream.seek(offsets[i], SEEK_SET);

		if (_flags & FLAG_FILLBG) {
			memset(buffer, 0, _width * _height * sizeof(uint32_t));
		}

		try {
			substream = stream.readStream(offsets[i+1]-offsets[i]);
			decodeFrame(buffer, (uint32_t*)_palettes[variant],
				*substream);
			_textureIDs[fpos] = registerTexture(_width,
				_height, buffer);
		} catch (...) {
			for (i = 0; i < fpos; i++) {
				freeTexture(_textureIDs[i]);
			}

			_frames = 0;
			delete substream;
			delete[] buffer;
			throw;
		}

		delete substream;
	}

	delete[] buffer;
}

void Image::clear(void) {
	unsigned i;

	for (i = 0; i < _frames * _palcount; i++) {
		freeTexture(_textureIDs[i]);
	}

	for (i = 0; i < _palcount; i++) {
		delete[] _palettes[i];
	}

	delete[] _textureIDs;
	delete[] _palettes;
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

unsigned Image::variantCount(void) const {
	return _palcount;
}

unsigned Image::textureID(unsigned frame) const {
	if (frame >= _frames * _palcount) {
		throw std::out_of_range("Image frame ID out of range");
	}

	return _textureIDs[frame];
}

const uint8_t *Image::palette(unsigned id) const {
	if (id >= _palcount) {
		throw std::out_of_range("Image palette ID out of range");
	}

	return _palettes[id];
}

void Image::draw(int x, int y, unsigned frame) const {
	drawTexture(textureID(frame), x, y);
}

void Image::drawCentered(int x, int y, unsigned frame) const {
	draw(x - _width / 2, y - _height / 2, frame);
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
	uint8_t shadowpal[3*4] = {TRANSPARENT, TRANSPARENT, SRGB(0x101018)};
	uint8_t outlinepal[3*4] = {TRANSPARENT, SRGB(0x101018), SRGB(0x101018)};

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

unsigned Font::textWidth(const char *str, unsigned charSpacing) const {
	unsigned idx, ret = 0;

	while ((idx = (unsigned char)*str++)) {
		if (idx >= _glyphCount) {
			continue;
		}

		ret += _glyphs[idx].width + charSpacing;
	}

	return ret ? ret - charSpacing : 0;
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
	return x + _glyphs[idx].width;
}

int Font::renderText(int x, int y, unsigned color, const char *str,
	unsigned outline, unsigned charSpacing) {
	for (; *str; str++) {
		x = renderChar(x, y, color, *str, outline) + charSpacing;
	}

	return x;
}

int Font::centerText(int x, int y, unsigned color, const char *str,
	unsigned outline, unsigned charSpacing) {
	return renderText(x - textWidth(str, charSpacing) / 2, y, color, str,
		outline, charSpacing);
}

const uint8_t *Font::titlePalette(unsigned color) {
	if (color >= TITLE_COLOR_MAX) {
		throw std::out_of_range("Invalid title font color ID");
	}

	return title_palettes[color];
}

const uint8_t *Font::fontPalette(unsigned color) {
	if (color >= FONT_COLOR_MAX) {
		throw std::out_of_range("Invalid font color ID");
	}

	return font_palettes[color];
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

Font *FontManager::fitFont(unsigned fontsize, unsigned maxwidth,
	const char *str) {
	Font *ret;

	fontsize++;

	do {
		fontsize--;
		ret = getFont(fontsize);
	} while (fontsize > 0 && ret->textWidth(str, 1) > maxwidth);

	return ret;
}

unsigned FontManager::fontCount(void) const {
	return _fontCount;
}

int fitText(int x, int y, unsigned maxwidth, unsigned fontsize, unsigned color,
	const char *str, unsigned outline, unsigned charSpacing) {
	unsigned width, len;
	Font *fnt = gameFonts->fitFont(fontsize, maxwidth, str);

	len = strlen(str);
	width = fnt->textWidth(str, 0);

	if (charSpacing * len > maxwidth - width) {
		charSpacing = (maxwidth - width) / len;
	}

	return fnt->renderText(x, y, color, str, outline, charSpacing);
}

int centerFitText(int x, int y, unsigned maxwidth, unsigned fontsize,
	unsigned color, const char *str, unsigned outline,
	unsigned charSpacing) {
	unsigned width, len;
	Font *fnt = gameFonts->fitFont(fontsize, maxwidth, str);

	len = strlen(str);
	width = fnt->textWidth(str, 0);

	if (charSpacing * len > maxwidth - width) {
		charSpacing = (maxwidth - width) / len;
	}

	return fnt->centerText(x, y, color, str, outline, charSpacing);
}

unsigned loopFrame(unsigned ticks, unsigned frametime, unsigned framecount) {
	return (ticks / frametime) % framecount;
}

unsigned bounceFrame(unsigned ticks, unsigned frametime, unsigned framecount) {
	unsigned ret;

	ret = (ticks / frametime) % (2 * framecount - 1);
	return ret < framecount ? ret : 2 * framecount - ret - 1;
}
