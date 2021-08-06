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

class MessageBoxWindow : public GuiWindow {
private:
	ImageAsset _header, _body, _footer;
	char *_text;

public:
	MessageBoxWindow(GuiView *parent, const char *text);
	~MessageBoxWindow(void);

	void redraw(unsigned curtick);
};

#endif
