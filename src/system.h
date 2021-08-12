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

#ifndef SYSTEM_H_
#define SYSTEM_H_

// Return the name of parent directory
char *parent_dir(const char *path);

// Create a single directory
void create_dir(const char *path);

// Create a whole path
void create_path(const char *path);

// Join two path segments using the appropriate directory separator
// Returns newly allocated string
char *concatPath(const char *basepath, const char *relpath);

// Find the correct name that can be used to open the file
// Returns newly allocated string, throws exception if no matching file exists
char *findDatadirFile(const char *filename);

// Return path to a file in data directory
// Returns newly allocated string
char *dataPath(const char *filename);

// Return path to a file in config directory
// Returns newly allocated string
char *configPath(const char *filename);

// Init relative datadir path on certain systems
void init_paths(const char *exepath);

// Free memory allocated by init_paths()
void cleanup_paths(void);

#endif
