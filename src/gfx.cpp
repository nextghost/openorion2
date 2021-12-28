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
#include "screen.h"

#define FLAG_JUNCTION	0x2000
#define FLAG_PALETTE	0x1000
#define FLAG_KEYCOLOR	0x0800
#define FLAG_FILLBG	0x0400
#define FLAG_NOCOMPRESS	0x0100

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

Font::Font(unsigned height) : _height(height), _glyphCount(0), _textureID(0),
	_glyphs(NULL) {

}

Font::~Font(void) {
	if (_glyphs) {
		freeTexture(_textureID);
		delete[] _glyphs;
	}
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

void Font::setPalette(const uint8_t *palette, unsigned colors) {
	setTexturePalette(_textureID, palette, 0, colors);
}

int Font::renderChar(int x, int y, char ch) {
	unsigned idx = (unsigned char)ch;

	if (idx >= _glyphCount) {
		return x;
	}

	drawTextureTile(_textureID, x, y, _glyphs[idx].offset, 0,
		_glyphs[idx].width, _height);
	return x + _glyphs[idx].width + 1;
}

int Font::renderText(int x, int y, const char *str) {
	for (; *str; str++) {
		x = renderChar(x, y, *str);
	}

	return x;
}

FontManager::FontManager(void) : _fonts(NULL), _fontCount(0), _size(0) {

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
	unsigned i, j, x, size, glyphCount, fontCount = 6;
	unsigned offsets[256], magic[4] = {25, 50, 10, 0x404032};
	Font *ptr = NULL;
	Font::Glyph *glyphs = NULL, *gptr = NULL;
	uint8_t *bitmap = NULL;
	uint8_t palette[PALSIZE] = {0};

	memset(palette + 4, 0xff, PALSIZE - 4);
	stream.seek(0, SEEK_SET);

	for (i = 0; i < 4; i++) {
		if (stream.readUint32LE() != magic[i]) {
			throw std::runtime_error("Invalid font header");
		}
	}

	// Create space for new fonts if needed
	if (_size < _fontCount + fontCount) {
		Font **tmp;

		size = _size + (_size > fontCount ? _size : fontCount);
		tmp = new Font*[size];

		if (_fonts) {
			memcpy(tmp, _fonts, _fontCount * sizeof(Font*));
			delete[] _fonts;
		}

		_fonts = tmp;
		_size = size;
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

		_fonts[_fontCount + i] = new Font(size);
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

			ptr->_textureID = registerTexture(size, ptr->height(),
				bitmap, palette, 0, 256);
			ptr->_glyphs = glyphs;
			ptr->_glyphCount = glyphCount;
			delete[] bitmap;
		}
	} catch (...) {
		fontCount -= i;

		for (i = 0; i < fontCount; i++) {
			delete _fonts[_fontCount + i];
		}

		delete[] glyphs;
		delete[] bitmap;
		throw;
	}
}

void FontManager::clear(void) {
	size_t i;

	for (i = 0; i < _fontCount; i++) {
		delete _fonts[i];
	}

	delete[] _fonts;
	_fonts = NULL;
	_fontCount = 0;
	_size = 0;
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
