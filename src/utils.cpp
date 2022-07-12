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

#include <cstddef>
#include <cstring>
#include <cctype>
#include "utils.h"

Recyclable *GarbageCollector::_garbage = NULL;
Mutex GarbageCollector::_garbageMutex;

Recyclable::Recyclable(void) : _nextGarbage(NULL) {

}

Recyclable::~Recyclable(void) {

}

void Recyclable::discard(void) {
	GarbageCollector::discard(this);
}

void GarbageCollector::discard(Recyclable *item) {
	if (!item) {
		return;
	}

	_garbageMutex.lock();
	item->_nextGarbage = _garbage;
	_garbage = item;
	_garbageMutex.unlock();
}

void GarbageCollector::flush(void) {
	Recyclable *cur, *next;

	_garbageMutex.lock();
	cur = _garbage;
	_garbage = NULL;
	_garbageMutex.unlock();

	for (; cur; cur = next) {
		next = cur->_nextGarbage;
		delete cur;
	}
}

char *copystr(const char *str) {
	char *ret = new char[strlen(str) + 1];

	strcpy(ret, str);
	return ret;
}

char *strlower(const char *str) {
	size_t i;
	char *ret = new char[strlen(str) + 1];

	for (i = 0; str[i]; i++) {
		ret[i] = tolower(str[i]);
	}

	ret[i] = '\0';
	return ret;
}

char *strupper(const char *str) {
	size_t i;
	char *ret = new char[strlen(str) + 1];

	for (i = 0; str[i]; i++) {
		ret[i] = toupper(str[i]);
	}

	ret[i] = '\0';
	return ret;
}

int isInRect(int x, int y, int rx, int ry, unsigned width, unsigned height) {
	return x >= rx && y >= ry && x < rx + (int)width &&
		y < ry + (int)height;
}
