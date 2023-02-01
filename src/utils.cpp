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

AutoMutex::AutoMutex(Mutex &m) : _mutex(m) {
	_mutex.lock();
}

AutoMutex::~AutoMutex(void) {
	_mutex.unlock();
}

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

StringBuffer::StringBuffer(size_t size) : _buf(NULL), _length(0), _size(size) {
	_size = _size < 32 ? 32 : _size;
	_buf = new char[_size];
	_buf[0] = '\0';
}

StringBuffer::StringBuffer(const char *str) : _buf(NULL), _length(0),
	_size(0) {

	_length = strlen(str);
	_size = _length >= 32 ? _length + 1 : 32;
	_buf = new char[_size];
	strcpy(_buf, str);
}

StringBuffer::StringBuffer(const StringBuffer &other) : _buf(NULL),
	_length(other._length), _size(other._length + 1) {

	_buf = new char[_size];
	strcpy(_buf, other._buf);
}

StringBuffer::~StringBuffer(void) {
	delete[] _buf;
}

void StringBuffer::resize(size_t newsize) {
	char *ptr;

	newsize++;

	if (newsize <= _size) {
		return;
	}

	newsize = newsize < 2 * _size ? 2 * _size : newsize;
	ptr = new char[newsize];
	strcpy(ptr, _buf);
	delete[] _buf;
	_buf = ptr;
}

void StringBuffer::vprintf(const char *fmt, va_list args) {
	int len;
	va_list tmpargs;

	va_copy(tmpargs, args);
	len = vsnprintf(_buf + _length, _size - _length, fmt, tmpargs);
	va_end(tmpargs);

	if (len < 0) {
		_buf[_length] = '\0';
		throw std::runtime_error("vsnprintf() failed");
	}

	while (_length + len >= _size) {
		_buf[_length] = '\0';
		resize(_length + len);
		va_copy(tmpargs, args);
		len = vsnprintf(_buf + _length, _size - _length, fmt, tmpargs);
		va_end(tmpargs);
	}

	_length += len;
}

const StringBuffer &StringBuffer::operator=(StringBuffer &other) {
	char *tmp = _buf;

	_buf = other._buf;
	_length = other._length;
	_size = other._size;
	other._buf = tmp;
	return *this;
}

const StringBuffer &StringBuffer::operator=(const char *str) {
	return truncate(0).append(str);
}

const StringBuffer &StringBuffer::operator+=(const StringBuffer &other) {
	return append(other._buf);
}

const StringBuffer &StringBuffer::operator+=(const char *str) {
	return append(str);
}

StringBuffer::operator const char *(void) const {
	return _buf;
}

StringBuffer &StringBuffer::append(const char *str) {
	size_t len = strlen(str);

	if (_length + len >= _size) {
		resize(_length + len);
	}

	strcpy(_buf + _length, str);
	_length += len;
	return *this;
}

StringBuffer &StringBuffer::append_printf(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);

	try {
		vprintf(fmt, args);
	} catch (...) {
		va_end(args);
		throw;
	}

	va_end(args);
	return *this;
}

StringBuffer &StringBuffer::append_ftime(const char *fmt,
	const struct tm *tbuf) {

	size_t len, fmtlen = strlen(fmt);

	len = strftime(_buf + _length, _size - _length, fmt, tbuf);

	while (!len && _size - _length < 64 * fmtlen) {
		resize(2 * _size);
		len = strftime(_buf + _length, _size - _length, fmt, tbuf);
	}

	_length += len;

	// just in case strftime() left the buffer in weird state...
	_buf[_length] = '\0';
	return *this;
}

StringBuffer &StringBuffer::printf(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	truncate(0);

	try {
		vprintf(fmt, args);
	} catch (...) {
		va_end(args);
		throw;
	}

	va_end(args);
	return *this;
}

StringBuffer &StringBuffer::ftime(const char *fmt, const struct tm *tbuf) {
	return truncate(0).append_ftime(fmt, tbuf);
}

StringBuffer &StringBuffer::truncate(size_t len) {
	_length = _length > len ? len : _length;
	_buf[_length] = '\0';
	return *this;
}


StringBuffer &StringBuffer::toLower(size_t start, ssize_t length) {
	size_t i, end = length >= 0 ? start + length : _length;

	if (start >= _length) {
		return *this;
	}

	for (i = start; i < end && _buf[i]; i++) {
		_buf[i] = tolower(_buf[i]);
	}

	return *this;
}

StringBuffer &StringBuffer::toUpper(size_t start, ssize_t length) {
	size_t i, end = length >= 0 ? start + length : _length;

	if (start >= _length) {
		return *this;
	}

	for (i = start; i < end && _buf[i]; i++) {
		_buf[i] = toupper(_buf[i]);
	}

	return *this;
}

ssize_t StringBuffer::find(char c) const {
	size_t i;

	for (i = 0; _buf[i]; i++) {
		if (_buf[i] == c) {
			return i;
		}
	}

	return -1;
}

size_t StringBuffer::length(void) const {
	return _length;
}

const char *StringBuffer::c_str(void) const {
	return _buf;
}

char *StringBuffer::copystr(void) const {
	char *ret = new char[_length + 1];

	strcpy(ret, _buf);
	return ret;
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

int checkBitfield(const uint8_t *bitfield, unsigned bit) {
	return bitfield && (bitfield[bit / 8] & (1 << (bit % 8)));
}
