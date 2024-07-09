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

#ifndef SDL_SCREEN_H_
#define SDL_SCREEN_H_

#include <cstdint>

// Logical screen size
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

class Screen {
protected:
	unsigned _width, _height;
	unsigned _clipX, _clipY, _clipW, _clipH;

	// Return pointer to 32bit xRGB pixel buffer for direct drawing
	virtual uint8_t *beginDraw(void) = 0;
	// Finish direct drawing into pixel buffer
	virtual void endDraw(void) = 0;
	// Length of pixel buffer line in bytes
	virtual unsigned drawPitch(void) const;

	// Apply current clip settings to parameters. Returns 1 if the clipped
	// rectangle is not empty, otherwise returns 0.
	int clipRect(int &x, int &y, unsigned &width, unsigned &height);

public:
	Screen(unsigned width, unsigned height);
	virtual ~Screen(void);

	unsigned width(void) const;
	unsigned height(void) const;

	// Refresh the screen using the last frame
	virtual void redraw(void) = 0;
	// Finish drawing a frame and copy it to screen
	virtual void update(void) = 0;

	// registerTexture() must return (nearly) consecutive texture IDs.
	// If the backend does not guarantee consecutive integers,
	// the implementation must maintain an internal translation table.
	virtual unsigned registerTexture(unsigned width, unsigned height,
		const uint32_t *data) = 0;
	virtual unsigned registerTexture(unsigned width, unsigned height,
		const uint8_t *data, const uint8_t *palette,
		unsigned firstcolor, unsigned colors) = 0;
	virtual void setTexturePalette(unsigned id, const uint8_t *palette,
		unsigned firstcolor, unsigned colors) = 0;
	virtual void freeTexture(unsigned id) = 0;

	// Draw whole texture
	virtual void drawTexture(unsigned id, int x, int y) = 0;

	// Draw part of texture specified by rectangle
	// (offsx, offsy) + (width, height)
	virtual void drawTextureTile(unsigned id, int x, int y, int offsx,
		int offsy, unsigned width, unsigned height) = 0;

	// Draw 8bit image data with palette
	virtual void drawBitmap(int x, int y, const uint8_t *image,
		unsigned width, unsigned height, const uint8_t *palette);
	virtual void drawBitmapTile(int x, int y, const uint8_t *image,
		unsigned offsx, unsigned offsy, unsigned width, unsigned height,
		unsigned pitch, const uint8_t *palette);

	virtual void drawLine(int x1, int y1, int x2, int y2, uint8_t r,
		uint8_t g, uint8_t b);

	virtual void fillRect(int x, int y, unsigned width, unsigned height,
		uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) = 0;
	virtual void drawRect(int x, int y, unsigned width, unsigned height,
		uint8_t r = 0, uint8_t g = 0, uint8_t b = 0,
		unsigned thickness = 1);
	virtual void fillTransparentRect(int x, int y, unsigned width,
		unsigned height, uint8_t a = 0xff, uint8_t r = 0,
		uint8_t g = 0, uint8_t b = 0);
	virtual void clear(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);

	// Set or remove clipping rectangle on the screen
	virtual void setClipRegion(unsigned x, unsigned y, unsigned width,
		unsigned height);
	virtual void unsetClipRegion(void);

	static Screen *createScreen(void);
};

extern Screen *gameScreen;

// Main event loop
void main_loop(void);

#endif
