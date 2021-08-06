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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdexcept>

class Mutex {
private:
	struct MutexImpl *_mutex;

	// Do NOT implement
	Mutex(const Mutex &other);
	const Mutex &operator=(const Mutex &other);

public:
	Mutex(void);
	~Mutex(void);

	void lock(void);
	void unlock(void);

	// Returns 1 if the mutex was locked, 0 if it's unavailable, throws
	// exception on error.
	int try_lock(void);
};

// Base class for objects which need to be deleted while possibly still in use
// by the rendering thread.
class Recyclable {
private:
	Recyclable *_nextGarbage;

public:
	Recyclable(void);
	virtual ~Recyclable(void) = 0;

	virtual void discard(void);

	friend class GarbageCollector;
};

// Helper class for collecting discarded objects which cannot be deleted right
// away. The current implementation allows multiple threads to discard objects
// but only one rendering thread may keep referencing them after discarding.
class GarbageCollector {
private:
	static Recyclable *_garbage;
	static Mutex _garbageMutex;

public:
	// Discard object, any thread may call this
	static void discard(Recyclable *item);

	// Free all discarded objects. Only the rendering thread can call this
	// after it has stopped using all the stale references.
	static void flush(void);
};

template <class C> class BilistNode : public Recyclable {
private:
	BilistNode *_prev, *_next;
	int _discarded;

	// Do NOT implement
	BilistNode(const BilistNode &other);
	const BilistNode &operator=(const BilistNode &other);

public:
	C *data;

	BilistNode(void);
	~BilistNode(void);

	// Insert this instance into existing list before or after given node
	void insert_before(BilistNode *node);
	void insert_after(BilistNode *node);
	void unlink(void);

	// Unlink this node from parent list, then put it in the garbage
	// collector but keep references to _next and _prev in case another
	// thread is still using this object.
	void discard(void);

	BilistNode *prev(void);
	BilistNode *next(void);
};

template <class C>
BilistNode<C>::BilistNode(void) : _prev(NULL), _next(NULL), _discarded(0),
	data(NULL) {

}

template <class C>
BilistNode<C>::~BilistNode(void) {
	if (!_discarded) {
		unlink();
	}
}

template <class C>
void BilistNode<C>::insert_before(BilistNode<C> *node) {
	if (_discarded) {
		throw std::invalid_argument("Cannot insert a discarded node");
	} else if (!node) {
		throw std::invalid_argument("Cannot insert list node before NULL value");
	}

	unlink();
	_next = node;
	_prev = node->_prev;
	node->_prev = this;

	if (_prev) {
		_prev->_next = this;
	}
}

template <class C>
void BilistNode<C>::insert_after(BilistNode<C> *node) {
	if (_discarded) {
		throw std::invalid_argument("Cannot insert a discarded node");
	} else if (!node) {
		throw std::invalid_argument("Cannot append list node after NULL value");
	}

	unlink();
	_prev = node;
	_next = node->_next;
	node->_next = this;

	if (_next) {
		_next->_prev = this;
	}
}

template <class C>
void BilistNode<C>::unlink(void) {
	if (_prev) {
		_prev->_next = _next;
	}

	if (_next) {
		_next->_prev = _prev;
	}

	_next = NULL;
	_prev = NULL;
}

template <class C>
void BilistNode<C>::discard(void) {
	_discarded = 1;

	if (_prev) {
		_prev->_next = _next;
	}

	if (_next) {
		_next->_prev = _prev;
	}

	Recyclable::discard();
}

template <class C>
BilistNode<C> *BilistNode<C>::prev(void) {
	return _prev;
}

template <class C>
BilistNode<C> *BilistNode<C>::next(void) {
	return _next;
}

#endif
