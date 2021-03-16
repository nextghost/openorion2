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

	unsigned i, tmp, palstart, palsize;
	size_t *offsets;
	uint32_t *buffer;

	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	stream.readUint16LE();
	_frames = stream.readUint16LE();
	_frametime = stream.readUint16LE();
	_flags = stream.readUint16LE();
	printf("Image: %ux%u, flags: %x\n", _width, _height, _flags);
	printf("%u frames @%d fps\n", _frames, _frametime ? 1000/_frametime:0);

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

	if (offsets[_frames] != stream.size()) {
		delete[] offsets;
		throw std::runtime_error("Image data size mismatch");
	}

	_palette = new uint8_t[1024];

	if (base_palette) {
		memcpy(_palette, base_palette, 1024);
	} else {
		memset(_palette, 0, 1024);
	}

	if (_flags & FLAG_PALETTE) {
		palstart = stream.readUint16LE();
		palsize = stream.readUint16LE();

		if (palstart + palsize > 256) {
			delete[] offsets;
			delete[] _palette;
			throw std::runtime_error("Palette buffer overflow");
		}

		printf("Palette: %u colors starting at %d\n", palsize,
			palstart);
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
		MemoryReadStream *substream;

		stream.seek(offsets[i], SEEK_SET);

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
