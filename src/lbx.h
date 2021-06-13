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

#include <stdexcept>
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

class AssetManager;

template <class C> class AssetPointer {
private:
	AssetManager *_manager;
	C *_asset;

protected:
	AssetPointer(AssetManager *manager, C *asset);

public:
	AssetPointer(void);
	AssetPointer(const AssetPointer &other);
	~AssetPointer(void);

	const AssetPointer &operator=(const AssetPointer &other);
	C *operator->(void);

	// Explicit-only to avoid accidental asset release immediately after
	// load. Image *img = (Image*)gameAssets->getImage(...) will not work
	// because the AssetPointer gets destroyed before you can call
	// gameAssets->takeAsset(img) and the just-loaded asset will be freed
	// with it.
	explicit operator C*(void);

	friend class AssetManager;
};

typedef AssetPointer<Image> ImageAsset;

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
	ImageAsset getImage(const char *filename, unsigned id,
		const uint8_t *palette = NULL);

	// Bump the asset reference counter to ensure it does not get deleted
	// by another part of code. You must call freeAsset() later.
	// NULL values are silently ignored.
	void takeAsset(const Image *img);

	// Decrease the reference counter and delete the asset when it's
	// no longer in use. NULL values are silently ignored.
	void freeAsset(const Image *img);
};

template <class C>
AssetPointer<C>::AssetPointer(void) : _manager(NULL), _asset(NULL) {

}

template <class C>
AssetPointer<C>::AssetPointer(AssetManager *manager, C *asset) :
	_manager(manager), _asset(asset) {

	if (_asset) {
		if (!_manager) {
			throw std::invalid_argument("Non-null asset pointer needs manager");
		}

		_manager->takeAsset(asset);
	}
}

template <class C>
AssetPointer<C>::AssetPointer(const AssetPointer &other) :
	_manager(other._manager), _asset(other._asset) {

	if (_asset) {
		_manager->takeAsset(_asset);
	}
}

template <class C>
AssetPointer<C>::~AssetPointer(void) {
	if (_asset) {
		_manager->freeAsset(_asset);
	}
}

template <class C>
const AssetPointer<C> &AssetPointer<C>::operator=(const AssetPointer &other) {
	if (_asset) {
		_manager->freeAsset(_asset);
	}

	_manager = other._manager;
	_asset = other._asset;

	if (_asset) {
		_manager->takeAsset(_asset);
	}

	return *this;
}


template <class C>
C *AssetPointer<C>::operator->(void) {
	if (!_asset) {
		throw std::runtime_error("Member access on NULL asset pointer");
	}

	return _asset;
}

template <class C>
AssetPointer<C>::operator C*(void) {
	return _asset;
}

extern AssetManager *gameAssets;

#endif
