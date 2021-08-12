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

#include <libgen.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include "utils.h"
#include "system.h"

char *parent_dir(const char *path) {
	char *tmp, *ret;

	tmp = copystr(path);

	try {
		ret = copystr(dirname(tmp));
	} catch (...) {
		delete[] tmp;
		throw;
	}

	delete[] tmp;
	return ret;
}

void create_path(const char *path) {
	struct stat buf;
	char *parent;
	int ret;

	ret = stat(path, &buf);

	if (!ret) {
		if (S_ISDIR(buf.st_mode)) {
			return;
		}

		throw std::runtime_error("Path exists but is not a directory");
	}

	if (errno != ENOENT) {
		throw std::runtime_error("Could not create path");
	}

	parent = parent_dir(path);

	try {
		create_path(parent);
	} catch (...) {
		delete[] parent;
	}

	delete[] parent;
	create_dir(path);
}


