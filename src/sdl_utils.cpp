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

#include <SDL_mutex.h>
#include <stdexcept>
#include "utils.h"

struct MutexImpl {
	SDL_mutex *mutex;
};

Mutex::Mutex(void) : _mutex(new MutexImpl) {
	_mutex->mutex = SDL_CreateMutex();

	if (!_mutex->mutex) {
		delete _mutex;
		throw std::runtime_error("Could not initialize mutex");
	}
}

Mutex::~Mutex(void) {
	SDL_DestroyMutex(_mutex->mutex);
	delete _mutex;
}

void Mutex::lock(void) {
	if (SDL_LockMutex(_mutex->mutex)) {
		throw std::runtime_error("Failed to lock mutex");
	}
}

void Mutex::unlock(void) {
	if (SDL_UnlockMutex(_mutex->mutex)) {
		throw std::runtime_error("Failed to unlock mutex");
	}
}

int Mutex::try_lock(void) {
	int ret = SDL_TryLockMutex(_mutex->mutex);

	if (ret && ret != SDL_MUTEX_TIMEDOUT) {
		throw std::runtime_error("Error when trying to lock mutex");
	}

	return !ret;
}
