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

#include <stdexcept>
#include <cstring>
#include "system.h"
#include "lbx.h"

#define LBX_MAGIC 0xfead

LBXArchive::LBXArchive(const char *filename) : _file(), _assetCount(0),
	_index(NULL) {

	unsigned magic, i;
	size_t offset;

	if (!_file.open(filename)) {
		throw std::runtime_error("Cannot open LBX file");
	}

	_assetCount = _file.readUint16LE();
	magic = _file.readUint16LE();

	if (!_assetCount || magic != LBX_MAGIC) {
		throw std::runtime_error("Invalid LBX file");
	}

	_file.readUint32LE();
	offset = _file.readUint32LE();
	_index = new LBXEntry[_assetCount];

	for (i = 0; i < _assetCount; i++) {
		_index[i].offset = offset;
		offset = _file.readUint32LE();

		if (offset <= _index[i].offset) {
			delete[] _index;
			throw std::runtime_error("Invalid LBX entry offset");
		}

		_index[i].size = offset - _index[i].offset;
	}
}

LBXArchive::~LBXArchive(void) {
	delete[] _index;
	_file.close();
}

const char *LBXArchive::filename(void) const {
	return _file.getName();
}

unsigned LBXArchive::assetCount(void) const {
	return _assetCount;
}

MemoryReadStream *LBXArchive::loadAsset(unsigned id) {
	if (id >= _assetCount) {
		throw std::out_of_range("Invalid LBX asset ID");
	}

	_file.seek(_index[id].offset, SEEK_SET);
	return _file.readStream(_index[id].size);
}

AssetManager::AssetManager(void) : _curfile(NULL), _cache(NULL),
	_cacheCount(0), _cacheSize(32), _imgLookupSize(32) {

	_cache = new FileCache[_cacheSize];
	memset(_cache, 0, _cacheSize * sizeof(FileCache));

	try {
		_imageLookup = new CacheEntry<Image>*[_imgLookupSize];
	} catch (...) {
		delete[] _cache;
		throw;
	}

	memset(_imageLookup, 0, _imgLookupSize * sizeof(size_t));
}

AssetManager::~AssetManager(void) {
	size_t i, j;

	for (i = 0; i < _cacheCount; i++) {
		for (j = 0; j < _cache[i].size; j++) {
			delete _cache[i].images[j].data;
		}

		delete[] _cache[i].filename;
		delete[] _cache[i].images;
	}

	delete _curfile;
	delete[] _cache;
	delete[] _imageLookup;
}

AssetManager::FileCache *AssetManager::getCache(const char *filename) {
	size_t i = 0, j = _cacheCount, tmp;
	int dir;
	char *realname;

	while (i < j) {
		tmp = (i + j) / 2;
		dir = strcasecmp(filename, _cache[tmp].filename);

		if (!dir) {
			return _cache + tmp;
		} else if (dir > 0) {
			i = tmp + 1;
		} else {
			j = tmp;
		}
	}

	realname = findDatadirFile(filename);

	if (_cacheCount >= _cacheSize) {
		size_t size = 2 * _cacheSize;
		FileCache *ptr;

		try {
			ptr = new FileCache[size];
		} catch (...) {
			delete[] realname;
			throw;
		}

		memcpy(ptr, _cache, i * sizeof(FileCache));
		memcpy(ptr+i+1, _cache+i, (_cacheCount-i) * sizeof(FileCache));
		delete[] _cache;
		_cache = ptr;
		_cacheSize = size;
	} else {
		for (j = _cacheCount; j > i; j--) {
			_cache[j] = _cache[j - 1];
		}
	}

	_cacheCount++;
	_cache[i].filename = realname;
	_cache[i].size = 0;
	_cache[i].images = NULL;
	return _cache + i;
}

void AssetManager::openArchive(FileCache *entry) {
	LBXArchive *archive;
	char *path;

	if (_curfile && !strcmp(entry->filename, _curfile->filename())) {
		return;
	}

	path = dataPath(entry->filename);

	try {
		archive = new LBXArchive(path);
	} catch (...) {
		delete[] path;
		throw;
	}

	delete[] path;
	delete _curfile;
	_curfile = archive;

	if (!entry->images) {
		size_t size = _curfile->assetCount();

		entry->images = new CacheEntry<Image>[size];
		memset(entry->images, 0, size * sizeof(CacheEntry<Image>));
		entry->size = size;
	}
}

ImageAsset AssetManager::getImage(const char *filename, unsigned id,
	const uint8_t *palette) {

	FileCache *entry;
	MemoryReadStream *stream;
	Image *img = NULL;
	unsigned texid;

	entry = getCache(filename);

	if (entry->images && id < entry->size && entry->images[id].data) {
		return ImageAsset(this, entry->images[id].data);
	}

	openArchive(entry);

	if (id >= entry->size) {
		throw std::out_of_range("Invalid asset ID");
	}

	stream = _curfile->loadAsset(id);

	try {
		img = new Image(*stream, palette);
		texid = img->textureID(0);

		if (texid >= _imgLookupSize) {
			CacheEntry<Image> **lookup = NULL;
			size_t size;

			size = _imgLookupSize;
			size = (2 * size > texid) ? 2 * size : (texid + 1);
			lookup = new CacheEntry<Image>*[size];
			memcpy(lookup, _imageLookup,
				_imgLookupSize * sizeof(*lookup));
			memset(lookup + _imgLookupSize, 0,
				(size - _imgLookupSize) * sizeof(*lookup));
			delete[] _imageLookup;
			_imageLookup = lookup;
			_imgLookupSize = size;
		}
	} catch (...) {
		delete img;
		delete stream;
		throw;
	}


	delete stream;
	entry->images[id].data = img;
	entry->images[id].refs = 0;
	_imageLookup[texid] = entry->images + id;
	return ImageAsset(this, entry->images[id].data);
}

void AssetManager::takeAsset(const Image *img) {
	unsigned texid;

	if (!img) {
		return;
	}

	texid = img->textureID(0);

	if (texid >= _imgLookupSize || !_imageLookup[texid]) {
		throw std::runtime_error("The image does not belong to this asset manager");
	}

	_imageLookup[texid]->refs++;
}

void AssetManager::freeAsset(const Image *img) {
	unsigned texid;
	CacheEntry<Image> *entry;

	if (!img) {
		return;
	}

	texid = img->textureID(0);

	if (texid >= _imgLookupSize || !_imageLookup[texid]) {
		throw std::runtime_error("The image does not belong to this asset manager");
	}

	entry = _imageLookup[texid];

	if (!--entry->refs) {
		delete entry->data;
		entry->data = NULL;
		_imageLookup[texid] = NULL;
	}
}

MemoryReadStream *AssetManager::rawData(const char *filename, unsigned id) {
	FileCache *entry;
	MemoryReadStream *stream;

	entry = getCache(filename);
	openArchive(entry);

	if (id >= entry->size) {
		throw std::out_of_range("Invalid asset ID");
	}

	return _curfile->loadAsset(id);
}

void load_fonts(const char *filename) {
	MemoryReadStream *stream;

	stream = gameAssets->rawData(filename, 0);

	try {
		gameFonts.clear();
		gameFonts.loadFonts(*stream);
	} catch (...) {
		delete stream;
		throw;
	}

	delete stream;
}
