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

#ifndef LBX_H_
#define LBX_H_

#include "stream.h"

class LBXArchive {
private:
	struct LBXEntry {
		size_t offset, size;
	};

	File _file;
	unsigned _assetCount;
	struct LBXEntry *_index;

	// Do NOT implement
	LBXArchive(const LBXArchive &other);
	const LBXArchive &operator=(const LBXArchive &other);

public:
	explicit LBXArchive(const char *filename);
	~LBXArchive(void);

	MemoryReadStream *loadAsset(unsigned id);
};

#endif
