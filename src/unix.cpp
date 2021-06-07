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

#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <dirent.h>
#include <stdexcept>
#include "system.h"

char *concatPath(const char *basepath, const char *relpath) {
	size_t baselen, pathlen;
	char *ret;

	baselen = strlen(basepath);
	pathlen = relpath ? strlen(relpath) : 0;
	ret = new char[baselen + pathlen + 2];
	strcpy(ret, basepath);

	if (baselen && ret[baselen - 1] != '/') {
		ret[baselen++] = '/';
	}

	strcpy(ret + baselen, relpath);
	return ret;
}

char *findDatadirFile(const char *filename) {
	DIR *dptr;
	struct dirent *entry;
	char *ret = NULL;
	int err;

	dptr = opendir(DATADIR);

	if (!dptr) {
		throw std::runtime_error("Failed to open data directory");
	}

	errno = 0;

	while ((entry = readdir(dptr))) {
		if (strcasecmp(filename, entry->d_name)) {
			continue;
		}

		try {
			ret = new char[strlen(entry->d_name) + 1];
		} catch (...) {
			closedir(dptr);
			throw;
		}

		strcpy(ret, entry->d_name);
		closedir(dptr);
		return ret;
	}

	err = errno;
	closedir(dptr);

	if (err) {
		throw std::runtime_error("Error reading data directory");
	}

	throw std::runtime_error("File not found");
}

char *dataPath(const char *filename) {
	return concatPath(DATADIR, filename);
}

void init_datadir(const char *exepath) {

}

void cleanup_datadir(void) {

}
