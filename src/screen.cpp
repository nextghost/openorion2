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

unsigned Screen::width(void) const {
	return _width;
}

unsigned Screen::height(void) const {
	return _height;
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
