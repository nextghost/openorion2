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
#include <cstdlib>
#include <stdexcept>
#include <SDL2/SDL.h>
#include "screen.h"
#include "gfx.h"
#include "lbx.h"

void main_loop(const char *filename, unsigned asset) {
	SDL_Event ev;
	LBXArchive lbx(filename);
	MemoryReadStream *stream; 
	unsigned frame = 0, frametime, start;

	stream = lbx.loadAsset(asset);
	Image img(*stream);
	delete stream;

	start = SDL_GetTicks();
	frametime = img.frameTime();
	frametime = frametime > 10 ? frametime : 200;

	while (1) {
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				return;
			}
		}

		frame = (SDL_GetTicks() - start) / frametime;
		render(img.textureID(frame % img.frameCount()));
		SDL_Delay(10);
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s lbx_file asset_id\n", argv[0]);
		return 1;
	}

	try {
		initScreen();
		main_loop(argv[1], atoi(argv[2]));
		shutdownScreen();
	} catch(std::exception &e) {
		fprintf(stderr, "Error: %s\n", e.what());
		return 1;
	}

	return 0;
}
