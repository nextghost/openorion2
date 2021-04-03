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

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "system.h"

static char *copystr(const char *str) {
	char *ret = new char[strlen(str) + 1];

	strcpy(ret, str);
	return ret;
}

char *concatPath(const char *basepath, const char *relpath) {
	size_t baselen;
	char *ret;

	baselen = strlen(basepath);
	ret = new char[baselen + strlen(relpath) + 2];
	strcpy(ret, basepath);

	if (baselen) {
		ret[baselen++] = '\\';
	}

	strcpy(ret + baselen, relpath);
	return ret;
}

char *findDatadirFile(const char *filename) {
	FILE *fr;

	fr = fopen(filename, "r");

	if (!fr) {
		throw std::runtime_error("File not found");
	}

	fclose(fr);
	return copystr(filename);
}

char *dataPath(const char *filename) {
	return copystr(filename);
}
