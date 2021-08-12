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
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <stdexcept>
#include "system.h"

void create_dir(const char *path) {
	if (mkdir(path, 0755)) {
		throw std::runtime_error("Could not create directory");
	}
}

char *concatPath(const char *basepath, const char *relpath) {
	size_t baselen, pathlen;
	char *ret;

	baselen = strlen(basepath);
	pathlen = relpath ? strlen(relpath) : 0;
	ret = new char[baselen + pathlen + 2];
	strcpy(ret, basepath);

	if (!relpath || !*relpath) {
		return ret;
	}

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

char *configPath(const char *filename) {
	struct passwd *user;
	const char *basedir;
	char *tmp = NULL, *ret = NULL;

	basedir = getenv("HOME");

	if (!basedir || !*basedir) {
		user = getpwuid(getuid());

		if (user) {
			basedir = user->pw_dir;
		}
	}

	if (!basedir || !*basedir) {
		throw std::runtime_error("Cannot find user's home directory");
	}

	try {
		ret = concatPath(basedir, ".config");
		tmp = concatPath(ret, "openorion2");
		delete[] ret;
		ret = NULL;
		ret = concatPath(tmp, filename);
	} catch (...) {
		delete[] ret;
		delete[] tmp;
		throw;
	}

	delete[] tmp;
	return ret;
}

void init_paths(const char *exepath) {
	char *tmp = configPath(NULL);

	try {
		create_path(tmp);
	} catch (...) {
		delete[] tmp;
		throw;
	}

	delete[] tmp;
}

void cleanup_paths(void) {

}
