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

#endif
