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

#include <cstring>
#include "screen.h"
#include "guimisc.h"

#define TEXTBOX_ARCHIVE "textbox.lbx"
#define ASSET_TEXTBOX_HEADER 0
#define ASSET_TEXTBOX_BODY 1
#define ASSET_TEXTBOX_FOOTER 2
#define ASSET_TEXTBOX_BUTTON 3

#define ERROR_ARCHIVE "warning.lbx"
#define ASSET_ERROR_BACKGROUND 0

MessageBoxWindow::MessageBoxWindow(GuiView *parent, const char *text) :
	GuiWindow(parent) {

	Widget *w = NULL;
	Font *fnt = gameFonts.getFont(3);

	_header = gameAssets->getImage(TEXTBOX_ARCHIVE, ASSET_TEXTBOX_HEADER);
	_body = gameAssets->getImage(TEXTBOX_ARCHIVE, ASSET_TEXTBOX_BODY,
		_header->palette());
	_footer = gameAssets->getImage(TEXTBOX_ARCHIVE, ASSET_TEXTBOX_FOOTER,
		_header->palette());
	_width = _header->width();
	_height = _header->height() + _footer->height() + fnt->height();
	_x = (SCREEN_WIDTH - _width) / 2;
	_y = (SCREEN_HEIGHT - _height) / 2;
	_text = copystr(text);

	try {
		w = createWidget(158, _height - 27, 64, 19);
		w->setClickSprite(MBUTTON_LEFT, TEXTBOX_ARCHIVE,
			ASSET_TEXTBOX_BUTTON, _header->palette(), 1);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod<GuiWindow>(*this, &MessageBoxWindow::close));
	} catch (...) {
		delete _text;
		clearWidgets();
		throw;
	}
}

MessageBoxWindow::~MessageBoxWindow(void) {
	delete[] _text;
}

void MessageBoxWindow::redraw(unsigned curtick) {
	Font *fnt;
	unsigned y, by = _header->height(), fh = _footer->height();
	uint8_t palette[] = {0, 0, 0, 0, 255, 48, 40, 4, 255, 32, 156, 28,
		255, 24, 120, 20};

	fillRect(_x + 9, _y + 9, _width - 18, _height - 40, 16, 16, 24);

	for (y = 10; y < _height - 31; y += 3) {
		fillRect(_x + 9, _y + y, _width - 18, 1, 36, 36, 40);
	}

	_header->draw(_x, _y);
	drawTextureTile(_body->textureID(0), _x, _y + by, 0, 0, _width,
		_height - by - fh);
	_footer->draw(_x, _y + _height - fh);
	fnt = gameFonts.getFont(3);
	fnt->setPalette(palette, 4);
	fnt->renderText(_x + 20, _y + 30, _text);
	redrawWidgets(_x, _y, curtick);
}

ErrorWindow::ErrorWindow(GuiView *parent, const char *text) :
	GuiWindow(parent, WINDOW_MODAL), _animStart(0) {

	_bg = gameAssets->getImage(ERROR_ARCHIVE, ASSET_ERROR_BACKGROUND);
	_width = _bg->width();
	_height = _bg->height();
	_x = (SCREEN_WIDTH - _width) / 2;
	_y = (SCREEN_HEIGHT - _height) / 2;

	_text = copystr(text);
}

ErrorWindow::~ErrorWindow(void) {
	delete[] _text;
}

void ErrorWindow::redraw(unsigned curtick) {
	unsigned frame, fcount;
	Font *fnt;
	int x, y;
	uint8_t palette[] = {0, 0, 0, 0, 255, 88, 12, 8, 255, 204, 24, 20,
		255, 144, 20, 28};

	if (!_animStart) {
		_animStart = curtick;
	}

	fnt = gameFonts.getFont(4);
	fnt->setPalette(palette, 4);
	fcount = _bg->frameCount();
	// FIXME: calculate frame time from image header
	// Original game plays 14 frames in ~1.3 seconds
	frame = (curtick - _animStart) / 100;
	frame %= 2 * fcount - 1;
	_bg->draw(_x, _y, frame >= fcount ? 2 * fcount - frame - 1: frame);
	x = _x + (_width - fnt->textWidth(_text)) / 2;
	y = _y + (_height - fnt->height()) / 2;
	fnt->renderText(x, y, _text);
}

void ErrorWindow::handleMouseUp(int x, int y, unsigned button) {
	close();
}
