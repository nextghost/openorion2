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

#ifndef GUIMISC_H_
#define GUIMISC_H_

#include "lbx.h"
#include "gui.h"

#define STUB(view) { new MessageBoxWindow(view, "Not implemented yet"); }

class MessageBoxWindow : public GuiWindow {
private:
	ImageAsset _header, _body, _footer;
	TextLayout _text;

	void initAssets(void);
	void initWidgets(void);

public:
	MessageBoxWindow(GuiView *parent, const char *text,
		unsigned flags = WINDOW_MOVABLE | WINDOW_MODAL);
	MessageBoxWindow(GuiView *parent, const char *title, const char *text,
		unsigned flags = WINDOW_MOVABLE | WINDOW_MODAL);
	MessageBoxWindow(GuiView *parent, unsigned help_id,
		const uint8_t *palette,
		unsigned flags = WINDOW_MOVABLE | WINDOW_MODAL);
	~MessageBoxWindow(void);

	void redraw(unsigned curtick);
};

class ConfirmationWindow : public GuiWindow {
private:
	ImageAsset _bg;
	GuiCallback _onYes, _onNo;
	TextLayout _text;

	void initWidgets(void);

public:
	ConfirmationWindow(GuiView *parent, const char *text,
		unsigned flags = WINDOW_MOVABLE | WINDOW_MODAL);

	void setYesCallback(const GuiCallback &callback);
	void setNoCallback(const GuiCallback &callback);

	void clickYes(int x, int y, int arg);
	void clickNo(int x, int y, int arg);

	void redraw(unsigned curtick);
};

class ErrorWindow : public GuiWindow {
private:
	ImageAsset _bg;
	unsigned _animStart;
	TextLayout _text;

public:
	ErrorWindow(GuiView *parent, const char *text);
	~ErrorWindow(void);

	void redraw(unsigned curtick);
	void handleMouseUp(int x, int y, unsigned button);
};

// colors must point to array of 4 RGB triplets
void drawFrame(int x, int y, unsigned width, unsigned height,
	const uint8_t *colors);
void drawETA(int x, int y, unsigned color, const char *str);

unsigned spriteSpacing(unsigned maxWidth, unsigned spriteWidth, unsigned count,
	unsigned maxSpace);

#endif
