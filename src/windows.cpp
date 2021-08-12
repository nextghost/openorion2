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

#include <direct.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "system.h"
#include "utils.h"

static char *data_basepath = NULL;

void create_dir(const char *path) {
	if (mkdir(path)) {
		throw std::runtime_error("Could not create directory");
	}
}

char *concatPath(const char *basepath, const char *relpath) {
	size_t i, baselen, pathlen;
	char *ret;

	baselen = strlen(basepath);
	pathlen = relpath ? strlen(relpath) : 0;
	ret = new char[baselen + pathlen + 2];
	strcpy(ret, basepath);

	if (relpath && *relpath) {
		if (baselen && ret[baselen - 1] != '/' &&
			ret[baselen - 1] != '\\') {
			ret[baselen++] = '\\';
		}

		strcpy(ret + baselen, relpath);
	}

	for (i = 0; ret[i]; i++) {
		if (ret[i] == '/') {
			ret[i] = '\\';
		}
	}

	return ret;
}

char *findDatadirFile(const char *filename) {
	char *path = dataPath(filename);
	FILE *fr;

	fr = fopen(path, "r");
	delete[] path;

	if (!fr) {
		throw std::runtime_error("File not found");
	}

	fclose(fr);
	return copystr(filename);
}

char *dataPath(const char *filename) {
	return concatPath(data_basepath, filename);
}

char *configPath(const char *filename) {
	const char *basedir;
	char *tmp = NULL, *ret = NULL;

	basedir = getenv("APPDATA");

	if (!basedir || !*basedir) {
		return dataPath(filename);
	}

	try {
		tmp = concatPath(basedir, "openorion2");
		ret = concatPath(tmp, filename);
	} catch (...) {
		delete[] tmp;
		throw;
	}

	return ret;
}

void init_paths(const char *exepath) {
	size_t i;
	const char *basepath = ".";
	char *tmp;

	for (i = 0; exepath[i]; i++) {
		if (exepath[i] == '\\') {
			basepath = exepath;
			break;
		}
	}

	data_basepath = parent_dir(basepath);
	tmp = configPath(NULL);

	try {
		create_path(tmp);
	} catch (...) {
		delete[] tmp;
		throw;
	}

	delete[] tmp;
}

void cleanup_paths(void) {
	delete[] data_basepath;
}
