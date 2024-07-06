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

class SDLScreen : public Screen {
private:
	SDL_Window *_window = NULL;
	SDL_Renderer *_renderer = NULL;
	SDL_Texture *_framebuffer = NULL;
	SDL_Surface *_drawbuffer = NULL;
	Texture *_textures = NULL;
	size_t _texture_count = 0, _texture_max = 0;
	uint32_t _amask = 0, _rmask = 0, _gmask = 0, _bmask = 0;

	void resizeTextureRegistry(void);
	void cleanup(void);

protected:
	uint8_t *beginDraw(void);
	void endDraw(void);
	unsigned drawPitch(void) const;

public:
	SDLScreen(unsigned width, unsigned height);
	~SDLScreen(void);

	void redraw(void);
	void update(void);

	unsigned registerTexture(unsigned width, unsigned height,
		const uint32_t *data);
	unsigned registerTexture(unsigned width, unsigned height,
		const uint8_t *data, const uint8_t *palette,
		unsigned firstcolor, unsigned colors);
	void setTexturePalette(unsigned id, const uint8_t *palette,
		unsigned firstcolor, unsigned colors);
	void freeTexture(unsigned id);

	void drawTexture(unsigned id, int x, int y);
	void drawTextureTile(unsigned id, int x, int y, int offsx, int offsy,
		unsigned width, unsigned height);

	void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g,
		uint8_t b);

	void fillRect(int x, int y, unsigned width, unsigned height,
		uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);

	void setClipRegion(unsigned x, unsigned y, unsigned width,
		unsigned height);
	void unsetClipRegion(void);
};

void SDLScreen::resizeTextureRegistry(void) {
	Texture *tmp;
	size_t size = _texture_max * 2;

	tmp = new Texture[size];
	memcpy(tmp, _textures, _texture_count * sizeof(Texture));
	memset(tmp + _texture_count, 0,
		(size - _texture_count) * sizeof(Texture));
	delete[] _textures;
	_textures = tmp;
	_texture_max = size;
}

SDLScreen::SDLScreen(unsigned w, unsigned h) : Screen(w, h),
	_window(NULL), _renderer(NULL), _framebuffer(NULL), _drawbuffer(NULL),
	_textures(NULL), _texture_count(0), _texture_max(0), _amask(0),
	_rmask(0), _gmask(0), _bmask(0) {

	unsigned flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	uint8_t *ptr;

	SDL_Init(SDL_INIT_VIDEO);

	if (SDL_CreateWindowAndRenderer(w, h, flags, &_window,
		&_renderer)) {
		throw std::runtime_error("Cannot create game window");
	}

	if (SDL_RenderSetLogicalSize(_renderer, w, h)) {
		cleanup();
		throw std::runtime_error("Cannot initialize renderer");
	}

	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetWindowTitle(_window, WINDOW_TITLE);

	_framebuffer = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING, w, h);

	if (!_framebuffer) {
		cleanup();
		throw std::runtime_error("Cannot create framebuffer");
	}

	_texture_max = 256;
	_textures = new Texture[_texture_max];
	memset(_textures, 0, _texture_max * sizeof(Texture));

	ptr = (uint8_t*)&_amask;
	ptr[0] = 0xff;
	ptr = (uint8_t*)&_rmask;
	ptr[1] = 0xff;
	ptr = (uint8_t*)&_gmask;
	ptr[2] = 0xff;
	ptr = (uint8_t*)&_bmask;
	ptr[3] = 0xff;

	_drawbuffer = SDL_CreateRGBSurface(0, w, h, 32, _rmask, _gmask, _bmask,
		0);

	if (!_drawbuffer) {
		cleanup();
		throw std::runtime_error("Cannot create draw buffer");
	}
}

SDLScreen::~SDLScreen(void) {
	cleanup();
}

void SDLScreen::cleanup(void) {
	size_t i;

	for (i = 0; i < _texture_count; i++) {
		if (_textures[i].drawsurf) {
			SDL_FreeSurface(_textures[i].drawsurf);
		}

		if (_textures[i].palsurf) {
			SDL_FreeSurface(_textures[i].palsurf);
		}
	}

	delete[] _textures;

	if (_drawbuffer) {
		SDL_FreeSurface(_drawbuffer);
	}

	if (_framebuffer) {
		SDL_DestroyTexture(_framebuffer);
	}

	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
	}

	if (_window) {
		SDL_DestroyWindow(_window);
	}

	SDL_Quit();
}

uint8_t *SDLScreen::beginDraw(void) {
	SDL_LockSurface(_drawbuffer);
	return (uint8_t*)_drawbuffer->pixels;
}

void SDLScreen::endDraw(void) {
	SDL_UnlockSurface(_drawbuffer);
}

unsigned SDLScreen::drawPitch(void) const {
	return _drawbuffer->pitch;
}

void SDLScreen::redraw(void) {
	SDL_RenderClear(_renderer);
	SDL_RenderCopy(_renderer, _framebuffer, NULL, NULL);
	SDL_RenderPresent(_renderer);
	SDL_UpdateWindowSurface(_window);
}

void SDLScreen::update(void) {
	int pitch, access, w, h;
	uint32_t format;
	void *pixels;
	SDL_Surface *target;
	SDL_Rect dstpos = {0, 0, (int)width(), (int)height()};

	if (SDL_LockTexture(_framebuffer, NULL, &pixels, &pitch)) {
		return;
	}

	if (SDL_QueryTexture(_framebuffer, &format, &access, &w, &h)) {
		SDL_UnlockTexture(_framebuffer);
		return;
	}

	target = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h,
		pitch / w, pitch, format);

	if (!target) {
		SDL_UnlockTexture(_framebuffer);
		return;
	}

	SDL_BlitSurface(_drawbuffer, NULL, target, &dstpos);
	SDL_FreeSurface(target);
	SDL_UnlockTexture(_framebuffer);
	redraw();
}

unsigned SDLScreen::registerTexture(unsigned w, unsigned h,
	const uint32_t *data) {

	SDL_Surface *surf;
	uint8_t *pixptr;
	unsigned i;

	if (_texture_count >= _texture_max) {
		resizeTextureRegistry();
	}

	surf = SDL_CreateRGBSurface(0, w, h, 32, _rmask, _gmask, _bmask,
		_amask);

	if (!surf) {
		throw std::runtime_error("Cannot allocate new SDL surface");
	}

	if (SDL_LockSurface(surf)) {
		SDL_FreeSurface(surf);
		throw std::runtime_error("Cannot lock texture surface");
	}

	pixptr = (uint8_t*)surf->pixels;

	for (i = 0; i < h; i++, pixptr += surf->pitch, data += w) {
		memcpy(pixptr, data, w * sizeof(uint32_t));
	}

	SDL_UnlockSurface(surf);

	_textures[_texture_count].palsurf = NULL;
	_textures[_texture_count].drawsurf = surf;
	return _texture_count++;
}

unsigned SDLScreen::registerTexture(unsigned w, unsigned h, const uint8_t *data,
	const uint8_t *palette, unsigned firstcolor, unsigned colors) {

	SDL_Surface *surf;
	uint8_t *pixptr;
	unsigned i, texid;

	if (_texture_count >= _texture_max) {
		resizeTextureRegistry();
	}

	surf = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);

	if (!surf) {
		throw std::runtime_error("Cannot allocate new SDL surface");
	}

	if (SDL_LockSurface(surf)) {
		SDL_FreeSurface(surf);
		throw std::runtime_error("Cannot lock texture surface");
	}

	pixptr = (uint8_t*)surf->pixels;

	for (i = 0; i < h; i++, pixptr += surf->pitch, data += w) {
		memcpy(pixptr, data, w);
	}

	SDL_UnlockSurface(surf);
	_textures[_texture_count].palsurf = surf;
	_textures[_texture_count].drawsurf = NULL;
	texid = _texture_count++;

	try {
		setTexturePalette(texid, palette, firstcolor, colors);
	} catch (...) {
		SDL_FreeSurface(surf);
		_textures[texid].palsurf = NULL;
		_texture_count--;
		throw;
	}

	return texid;
}

void SDLScreen::setTexturePalette(unsigned id, const uint8_t *palette,
	unsigned firstcolor, unsigned colors) {

	unsigned i;
	SDL_Surface *surf;
	SDL_Color conv[256];

	if (id >= _texture_count) {
		throw std::out_of_range("Invalid texture ID");
	}

	if (firstcolor + colors > 256) {
		throw std::out_of_range("Palette segment out of range");
	}

	if (!_textures[id].palsurf || !_textures[id].palsurf->format->palette) {
		throw std::invalid_argument("Texture does not have a palette");
	}

	surf = _textures[id].palsurf;

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

	if (_textures[id].drawsurf) {
		SDL_FreeSurface(_textures[id].drawsurf);
	}

	_textures[id].drawsurf = surf;
}

void SDLScreen::freeTexture(unsigned id) {
	Texture *tex;

	if (id >= _texture_count) {
		return;
	}

	if (_textures[id].drawsurf) {
		SDL_FreeSurface(_textures[id].drawsurf);
		_textures[id].drawsurf = NULL;
	}

	if (_textures[id].palsurf) {
		SDL_FreeSurface(_textures[id].palsurf);
		_textures[id].palsurf = NULL;
	}

	tex = _textures + _texture_count;

	for (; _texture_count > 0; _texture_count--) {
		tex--;

		if (tex->palsurf || tex->drawsurf) {
			break;
		}
	}
}

void SDLScreen::drawTexture(unsigned id, int x, int y) {
	SDL_Rect dst = {x, y, 0, 0};

	if (id >= _texture_count || !_textures[id].drawsurf) {
		throw std::out_of_range("Invalid texture ID");
	}

	SDL_BlitSurface(_textures[id].drawsurf, NULL, _drawbuffer, &dst);
}

void SDLScreen::drawTextureTile(unsigned id, int x, int y, int offsx, int offsy,
	unsigned w, unsigned h) {
	SDL_Rect src = {offsx, offsy, (int)w, (int)h};
	SDL_Rect dst = {x, y, (int)w, (int)h};

	if (id >= _texture_count || !_textures[id].drawsurf) {
		throw std::out_of_range("Invalid texture ID");
	}

	SDL_BlitSurface(_textures[id].drawsurf, &src, _drawbuffer, &dst);
}

void SDLScreen::drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g,
	uint8_t b) {

	int x, y, dx = 1, dy = 1;
	unsigned xlen, ylen, steps, len, cur, i = 0;
	uint32_t color = SDL_MapRGB(_drawbuffer->format, r, g, b);
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

		SDL_FillRect(_drawbuffer, &rect, color);
		rect.x += dx;
		rect.y += dy;
	}
}

void SDLScreen::fillRect(int x, int y, unsigned w, unsigned h, uint8_t r,
	uint8_t g, uint8_t b) {
	SDL_Rect rect = {x, y, (int)w, (int)h};

	SDL_FillRect(_drawbuffer, &rect,
		SDL_MapRGB(_drawbuffer->format, r, g, b));
}

void SDLScreen::setClipRegion(unsigned x, unsigned y, unsigned w, unsigned h) {
	SDL_Rect rect = {(int)x, (int)y, (int)w, (int)h};

	SDL_SetClipRect(_drawbuffer, &rect);
	Screen::setClipRegion(x, y, w, h);
}

void SDLScreen::unsetClipRegion(void) {
	SDL_SetClipRect(_drawbuffer, NULL);
	Screen::unsetClipRegion();
}

Screen *Screen::createScreen(void) {
	return new SDLScreen(SCREEN_WIDTH, SCREEN_HEIGHT);
}
