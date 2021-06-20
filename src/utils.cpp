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
