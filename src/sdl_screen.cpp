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

#include <SDL2/SDL.h>
#include <stdexcept>
#include "screen.h"

SDL_Window *window = NULL;
SDL_Surface *wsurface = NULL, **textures = NULL;
size_t texture_count = 0, texture_max = 0;
uint32_t amask = 0, rmask = 0, gmask = 0, bmask = 0;

void initScreen(void) {
	uint8_t *ptr;

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("OpenOrion2", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	if (!window) {
		throw std::runtime_error("Cannot create game window");
	}

	wsurface = SDL_GetWindowSurface(window);

	if (!wsurface) {
		SDL_DestroyWindow(window);
		throw std::runtime_error("Cannot allocate rendering surface");
	}

	texture_max = 256;
	textures = new SDL_Surface*[texture_max];

	ptr = (uint8_t*)&amask;
	ptr[0] = 0xff;
	ptr = (uint8_t*)&rmask;
	ptr[1] = 0xff;
	ptr = (uint8_t*)&gmask;
	ptr[2] = 0xff;
	ptr = (uint8_t*)&bmask;
	ptr[3] = 0xff;
}

void shutdownScreen(void) {
	size_t i;

	for (i = 0; i < texture_count; i++) {
		if (textures[i]) {
			SDL_FreeSurface(textures[i]);
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
}

void updateScreen(void) {
	SDL_UpdateWindowSurface(window);
}

unsigned registerTexture(unsigned width, unsigned height, const uint32_t *data) {
	SDL_Surface *surf;
	uint8_t *pixptr;
	unsigned i;

	if (texture_count >= texture_max) {
		SDL_Surface **sptr;

		texture_max *= 2;
		sptr = new SDL_Surface*[texture_max];

		memcpy(sptr, textures, texture_count * sizeof(SDL_Surface*));

		delete[] textures;
		textures = sptr;
	}

	surf = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask,
		amask);

	if (!surf) {
		throw std::runtime_error("Cannot allocate new SDL surface");
	}

	if (SDL_LockSurface(surf)) {
		SDL_FreeSurface(surf);
		throw std::runtime_error("Cannot lock texture surface");
	}

	pixptr = (uint8_t*)surf->pixels;

	for (i = 0; i < height; i++, pixptr += surf->pitch, data += width) {
		memcpy(pixptr, data, width * sizeof(uint32_t));
	}
	
	SDL_UnlockSurface(surf);

	textures[texture_count] = surf;
	return texture_count++;
}

void freeTexture(unsigned id) {
	if (!textures[id]) {
		return;
	}

	SDL_FreeSurface(textures[id]);
	textures[id] = NULL;

	for (; texture_count > 0 && !textures[texture_count]; texture_count--);
}

// FIXME: replace with real rendering engine
void render(unsigned id) {
	SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	SDL_FillRect(wsurface, &rect, SDL_MapRGB(wsurface->format, 0, 0, 0));

	if (textures[id]) {
		SDL_BlitSurface(textures[id], NULL, wsurface, &rect);
	}

	SDL_UpdateWindowSurface(window);
}
