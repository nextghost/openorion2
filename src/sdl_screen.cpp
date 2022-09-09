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

#include <SDL.h>
#include <stdexcept>
#include "screen.h"

#define WINDOW_TITLE "OpenOrion2"

struct Texture {
	SDL_Surface *palsurf, *drawsurf;
};

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *framebuffer = NULL;
SDL_Surface *drawbuffer = NULL;
Texture *textures = NULL;
size_t texture_count = 0, texture_max = 0;
uint32_t amask = 0, rmask = 0, gmask = 0, bmask = 0;

static void resizeTextureRegistry(void) {
	Texture *tmp;
	size_t size = texture_max * 2;

	tmp = new Texture[size];
	memcpy(tmp, textures, texture_count * sizeof(Texture));
	memset(tmp + texture_count, 0, (size-texture_count) * sizeof(Texture));
	delete[] textures;
	textures = tmp;
	texture_max = size;
}

void initScreen(void) {
	unsigned flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	uint8_t *ptr;

	SDL_Init(SDL_INIT_VIDEO);

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, flags,
		&window, &renderer)) {
		throw std::runtime_error("Cannot create game window");
	}

	if (SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) {
		throw std::runtime_error("Cannot initialize renderer");
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetWindowTitle(window, WINDOW_TITLE);

	framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	if (!framebuffer) {
		throw std::runtime_error("Cannot create framebuffer");
	}

	texture_max = 256;
	textures = new Texture[texture_max];
	memset(textures, 0, texture_max * sizeof(Texture));

	ptr = (uint8_t*)&amask;
	ptr[0] = 0xff;
	ptr = (uint8_t*)&rmask;
	ptr[1] = 0xff;
	ptr = (uint8_t*)&gmask;
	ptr[2] = 0xff;
	ptr = (uint8_t*)&bmask;
	ptr[3] = 0xff;

	drawbuffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		rmask, gmask, bmask, 0);

	if (!drawbuffer) {
		throw std::runtime_error("Cannot create draw buffer");
	}
}

void shutdownScreen(void) {
	size_t i;

	for (i = 0; i < texture_count; i++) {
		if (textures[i].drawsurf) {
			SDL_FreeSurface(textures[i].drawsurf);
		}

		if (textures[i].palsurf) {
			SDL_FreeSurface(textures[i].palsurf);
		}
	}

	if (drawbuffer) {
		SDL_FreeSurface(drawbuffer);
	}

	if (framebuffer) {
		SDL_DestroyTexture(framebuffer);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	if (window) {
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
}

void redrawScreen(void) {
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_UpdateWindowSurface(window);
}

void updateScreen(void) {
	int pitch, access, width, height;
	uint32_t format;
	void *pixels;
	SDL_Surface *target;
	SDL_Rect dstpos = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	if (SDL_LockTexture(framebuffer, NULL, &pixels, &pitch)) {
		return;
	}

	if (SDL_QueryTexture(framebuffer, &format, &access, &width, &height)) {
		SDL_UnlockTexture(framebuffer);
		return;
	}

	target = SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height,
		pitch / width, pitch, format);

	if (!target) {
		SDL_UnlockTexture(framebuffer);
		return;
	}

	SDL_BlitSurface(drawbuffer, NULL, target, &dstpos);
	SDL_FreeSurface(target);
	SDL_UnlockTexture(framebuffer);
	redrawScreen();
}

unsigned registerTexture(unsigned width, unsigned height, const uint32_t *data) {
	SDL_Surface *surf;
	uint8_t *pixptr;
	unsigned i;

	if (texture_count >= texture_max) {
		resizeTextureRegistry();
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

	textures[texture_count].palsurf = NULL;
	textures[texture_count].drawsurf = surf;
	return texture_count++;
}

unsigned registerTexture(unsigned width, unsigned height, const uint8_t *data,
	const uint8_t *palette, unsigned firstcolor, unsigned colors) {

	SDL_Surface *surf;
	uint8_t *pixptr;
	unsigned i, texid;

	if (texture_count >= texture_max) {
		resizeTextureRegistry();
	}

	surf = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);

	if (!surf) {
		throw std::runtime_error("Cannot allocate new SDL surface");
	}

	if (SDL_LockSurface(surf)) {
		SDL_FreeSurface(surf);
		throw std::runtime_error("Cannot lock texture surface");
	}

	pixptr = (uint8_t*)surf->pixels;

	for (i = 0; i < height; i++, pixptr += surf->pitch, data += width) {
		memcpy(pixptr, data, width);
	}

	SDL_UnlockSurface(surf);
	textures[texture_count].palsurf = surf;
	textures[texture_count].drawsurf = NULL;
	texid = texture_count++;

	try {
		setTexturePalette(texid, palette, firstcolor, colors);
	} catch (...) {
		SDL_FreeSurface(surf);
		textures[texid].palsurf = NULL;
		texture_count--;
		throw;
	}

	return texid;
}

void setTexturePalette(unsigned id, const uint8_t *palette,
	unsigned firstcolor, unsigned colors) {

	unsigned i;
	SDL_Surface *surf;
	SDL_Color conv[256];

	if (id >= texture_count) {
		throw std::out_of_range("Invalid texture ID");
	}

	if (firstcolor + colors > 256) {
		throw std::out_of_range("Palette segment out of range");
	}

	if (!textures[id].palsurf || !textures[id].palsurf->format->palette) {
		throw std::invalid_argument("Texture does not have a palette");
	}

	surf = textures[id].palsurf;

	for (i = 0; i < colors; i++) {
		conv[i].a = palette[4 * i];
		conv[i].r = palette[4 * i + 1];
		conv[i].g = palette[4 * i + 2];
		conv[i].b = palette[4 * i + 3];
	}

	if (SDL_SetPaletteColors(surf->format->palette, conv, firstcolor,
		colors)) {
		throw std::runtime_error("Failed to modify texture palette");
	}

	surf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);

	if (!surf) {
		throw std::runtime_error("Failed to convert pixel format");
	}

	SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);

	if (textures[id].drawsurf) {
		SDL_FreeSurface(textures[id].drawsurf);
	}

	textures[id].drawsurf = surf;
}

void freeTexture(unsigned id) {
	Texture *tex;

	if (id >= texture_count) {
		return;
	}

	if (textures[id].drawsurf) {
		SDL_FreeSurface(textures[id].drawsurf);
		textures[id].drawsurf = NULL;
	}

	if (textures[id].palsurf) {
		SDL_FreeSurface(textures[id].palsurf);
		textures[id].palsurf = NULL;
	}

	tex = textures + texture_count;

	for (; texture_count > 0; texture_count--) {
		tex--;

		if (tex->palsurf || tex->drawsurf) {
			break;
		}
	}
}

void drawTexture(unsigned id, int x, int y) {
	SDL_Rect dst = {x, y, 0, 0};

	if (id >= texture_count || !textures[id].drawsurf) {
		throw std::out_of_range("Invalid texture ID");
	}

	SDL_BlitSurface(textures[id].drawsurf, NULL, drawbuffer, &dst);
}

void drawTextureTile(unsigned id, int x, int y, int offsx, int offsy,
	unsigned width, unsigned height) {
	SDL_Rect src = {offsx, offsy, (int)width, (int)height};
	SDL_Rect dst = {x, y, SCREEN_WIDTH, SCREEN_HEIGHT};

	if (id >= texture_count || !textures[id].drawsurf) {
		throw std::out_of_range("Invalid texture ID");
	}

	SDL_BlitSurface(textures[id].drawsurf, &src, drawbuffer, &dst);
}

void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b) {
	int x, y, dx = 1, dy = 1;
	unsigned xlen, ylen, steps, len, cur, i = 0;
	uint32_t color = SDL_MapRGB(drawbuffer->format, r, g, b);
	SDL_Rect rect = {x1, y1, 1, 1};

	xlen = (x1 < x2 ? x2 - x1 : x1 - x2) + 1;
	ylen = (y1 < y2 ? y2 - y1 : y1 - y2) + 1;

	if (xlen > ylen) {
		steps = ylen;
		len = xlen;
		x = x1 < x2 ? x1 : x2;
		y = x1 < x2 ? y1 : y2;
		dy = y1 < y2 ? 1 : -1;
		dy = x1 < x2 ? dy : -dy;
	} else {
		steps = xlen;
		len = ylen;
		x = y1 < y2 ? x1 : x2;
		y = y1 < y2 ? y1 : y2;
		dx = x1 < x2 ? 1 : -1;
		dx = y1 < y2 ? dx : -dx;
	}

	rect.x = x;
	rect.y = y;

	for (i = 0; i < steps; i++) {
		cur = (len * (i + 1)) / steps;

		if (xlen > ylen) {
			rect.w = dx = x + cur - rect.x;
		} else {
			rect.h = dy = y + cur - rect.y;
		}

		SDL_FillRect(drawbuffer, &rect, color);
		rect.x += dx;
		rect.y += dy;
	}
}

void drawRect(int x, int y, unsigned width, unsigned height, uint8_t r,
	uint8_t g, uint8_t b, unsigned thickness) {

	Uint32 color = SDL_MapRGB(drawbuffer->format, r, g, b);
	SDL_Rect rect = {x, y, (int)width, (int)thickness};

	if (width <= 2 * thickness || height <= 2 * thickness) {
		fillRect(x, y, width, height, r, g, b);
		return;
	}

	SDL_FillRect(drawbuffer, &rect, color);
	rect.y += height - thickness;
	SDL_FillRect(drawbuffer, &rect, color);
	rect.y = y + thickness;
	rect.w = thickness;
	rect.h = height - 2 * thickness;
	SDL_FillRect(drawbuffer, &rect, color);
	rect.x += width - thickness;
	SDL_FillRect(drawbuffer, &rect, color);
}

void fillRect(int x, int y, unsigned width, unsigned height, uint8_t r,
	uint8_t g, uint8_t b) {
	SDL_Rect rect = {x, y, (int)width, (int)height};

	SDL_FillRect(drawbuffer, &rect,
		SDL_MapRGB(drawbuffer->format, r, g, b));
}

void clearScreen(uint8_t r, uint8_t g, uint8_t b) {
	fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, r, g, b);
}

void setClipRegion(int x, int y, unsigned width, unsigned height) {
	SDL_Rect rect = {x, y, (int)width, (int)height};

	SDL_SetClipRect(drawbuffer, &rect);
}

void unsetClipRegion(void) {
	SDL_SetClipRect(drawbuffer, NULL);
}
