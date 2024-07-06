/*
 * This file is part of OpenOrion2
 * Copyright (C) 2024 Martin Doucha
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

#include "screen.h"

Screen::Screen(unsigned w, unsigned h) : _width(w), _height(h), _clipX(0),
	_clipY(0), _clipW(w), _clipH(h) {

}

Screen::~Screen(void) {

}

unsigned Screen::drawPitch(void) const {
	return 4 * _width;
}

unsigned Screen::width(void) const {
	return _width;
}

unsigned Screen::height(void) const {
	return _height;
}

int Screen::clipRect(int &x, int &y, unsigned &w, unsigned &h) {
	if (x + (int)w <= (int)_clipX || y + (int)h <= (int)_clipY ||
		x >= (int)(_clipX + _clipW) || y >= (int)(_clipY + _clipH)) {
		return 0;
	}

	if (x < (int)_clipX) {
		w -= _clipX - x;
		x = _clipX;
	}

	if (y < (int)_clipY) {
		h -= _clipY - y;
		y = _clipY;
	}

	if (x + w > (int)_clipX + _clipW) {
		w = _clipX + _clipW - x;
	}

	if (y + h > (int)_clipY + _clipH) {
		h = _clipY + _clipH - y;
	}

	return 1;
}

void Screen::drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g,
	uint8_t b) {
	int x, y, cx, cy, dx = 1, dy = 1;
	unsigned w = 1, h = 1, xlen, ylen, steps, len, cur, i = 0;

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

	cx = x;
	cy = y;

	for (i = 0; i < steps; i++) {
		cur = (len * (i + 1)) / steps;

		if (xlen > ylen) {
			w = dx = x + cur - cx;
		} else {
			h = dy = y + cur - cy;
		}

		fillRect(cx, cy, w, h, r, g, b);
		cx += dx;
		cy += dy;
	}
}

void Screen::drawRect(int x, int y, unsigned width, unsigned height, uint8_t r,
	uint8_t g, uint8_t b, unsigned thickness) {

	if (width <= 2 * thickness || height <= 2 * thickness) {
		fillRect(x, y, width, height, r, g, b);
		return;
	}

	fillRect(x, y, width, thickness, r, g, b);
	fillRect(x, y + thickness, thickness, height - 2 * thickness, r, g, b);
	fillRect(x + width - thickness, y + thickness, thickness,
		height - 2 * thickness, r, g, b);
	fillRect(x, y + height - thickness, width, thickness, r, g, b);
}

void Screen::fillTransparentRect(int x, int y, unsigned w, unsigned h,
	uint8_t a, uint8_t r, uint8_t g, uint8_t b) {

	unsigned i, j, pitch;
	uint32_t da, ra, ga, ba;
	uint8_t *dest, *drawbuf;

	if (!clipRect(x, y, w, h)) {
		return;
	}

	da = 0xff - a;
	ra = uint32_t(a) * r;
	ga = uint32_t(a) * g;
	ba = uint32_t(a) * b;

	drawbuf = beginDraw();
	pitch = drawPitch();

	for (i = 0; i < h; i++) {
		dest = drawbuf + (y + i) * pitch + x * 4;

		for (j = 0; j < w; j++, dest += 4) {
			dest[1] = (ra + dest[1] * da) / 0xff;
			dest[2] = (ga + dest[2] * da) / 0xff;
			dest[3] = (ba + dest[3] * da) / 0xff;
		}
	}

	endDraw();
}

void Screen::clear(uint8_t r, uint8_t g, uint8_t b) {
	fillRect(0, 0, _width, _height, r, g, b);
}

void Screen::setClipRegion(unsigned x, unsigned y, unsigned width,
	unsigned height) {
	_clipX = x;
	_clipY = y;
	_clipW = width;
	_clipH = height;
}

void Screen::unsetClipRegion(void) {
	_clipX = 0;
	_clipY = 0;
	_clipW = _width;
	_clipH = _height;
}
