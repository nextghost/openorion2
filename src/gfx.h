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

#endif
