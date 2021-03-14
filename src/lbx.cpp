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

MemoryReadStream *LBXArchive::loadAsset(unsigned id) {
	if (id >= _assetCount) {
		throw std::out_of_range("Invalid LBX asset ID");
	}

	_file.seek(_index[id].offset, SEEK_SET);
	return _file.readStream(_index[id].size);
}
