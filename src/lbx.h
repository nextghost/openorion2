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
#include "gfx.h"

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

	const char *filename(void) const;
	unsigned assetCount(void) const;

	MemoryReadStream *loadAsset(unsigned id);
};

class AssetManager {
private:
	template <class C> struct CacheEntry {
		C *data;
		unsigned refs;
	};

	struct FileCache {
		char *filename;
		size_t size;
		CacheEntry<Image> *images;
	};

	LBXArchive *_curfile;
	FileCache *_cache;

	// Lookup table that maps texture IDs to _cache image entries
	CacheEntry<Image> **_imageLookup;
	size_t _cacheCount, _cacheSize, _imgLookupSize;

protected:
	FileCache *getCache(const char *filename);
	void openArchive(FileCache *entry);

public:
	AssetManager(void);
	~AssetManager(void);

	// Get asset from cache, loading it from disk if necessary. Reference
	// counter gets automatically increased.
	Image *getImage(const char *filename, unsigned id,
		const uint8_t *palette = NULL);

	// Bump the asset reference counter to ensure it does not get deleted
	// by another part of code. You must call freeImage() later.
	// NULL values are silently ignored.
	void takeImage(const Image *img);

	// Decrease the reference counter and delete the asset when it's
	// no longer in use. NULL values are silently ignored.
	void freeImage(const Image *img);
};

extern AssetManager *gameAssets;

#endif
