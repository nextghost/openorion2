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

#include "lbx.h"
#include "guimisc.h"
#include "mainmenu.h"

#define MENU_ARCHIVE "mainmenu.lbx"
#define ASSET_MENU_BACKGROUND 21
#define ASSET_MENU_CONTINUE_ON 4
#define ASSET_MENU_CONTINUE_OFF 5
#define ASSET_MENU_LOAD_ON 7
#define ASSET_MENU_LOAD_OFF 8
#define ASSET_MENU_NEWGAME 10
#define ASSET_MENU_MULTIPLAYER 13
#define ASSET_MENU_SCORES 16
#define ASSET_MENU_QUIT 19

MainMenuView::MainMenuView(void) : _background() {
	Widget *w = NULL;
	const uint8_t *pal;

	_background = gameAssets->getImage(MENU_ARCHIVE, ASSET_MENU_BACKGROUND);
	pal = _background->palette();

	try {
		w = new Widget(415, 172, 153, 23);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickContinue));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_CONTINUE_ON,
			pal, 0);
		addWidget(w);
		w = NULL;

		w = new Widget(415, 195, 153, 22);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickLoad));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_LOAD_ON, pal, 0);
		addWidget(w);
		w = NULL;

		w = new Widget(415, 217, 153, 23);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickNew));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_NEWGAME, pal, 0);
		addWidget(w);
		w = NULL;

		w = new Widget(415, 240, 153, 22);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickMultiplayer));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_MULTIPLAYER,
			pal, 0);
		addWidget(w);
		w = NULL;

		w = new Widget(415, 262, 153, 23);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickScoreboard));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_SCORES, pal, 0);
		addWidget(w);
		w = NULL;

		w = new Widget(415, 285, 153, 22);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickQuit));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_QUIT, pal, 0);
		addWidget(w);
	} catch (...) {
		delete w;
		clearWidgets();
		throw;
	}
}

MainMenuView::~MainMenuView(void) {

}

void MainMenuView::redraw(unsigned curtick) {
	_background->draw(0, 0);
	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void MainMenuView::clickContinue(int x, int y, int arg) {
	TextBoxWindow *w;

	w = new TextBoxWindow(this, "Continue not implemented yet");
}

void MainMenuView::clickLoad(int x, int y, int arg) {
	TextBoxWindow *w;

	w = new TextBoxWindow(this, "Load Game not implemented yet");
}

void MainMenuView::clickNew(int x, int y, int arg) {
	TextBoxWindow *w;

	w = new TextBoxWindow(this, "New Game not implemented yet");
}

void MainMenuView::clickMultiplayer(int x, int y, int arg) {
	TextBoxWindow *w;

	w = new TextBoxWindow(this, "Multiplayer not implemented yet");
}

void MainMenuView::clickScoreboard(int x, int y, int arg) {
	TextBoxWindow *w;

	w = new TextBoxWindow(this, "Hall of Fame not implemented yet");
}

void MainMenuView::clickQuit(int x, int y, int arg) {
	gui_stack->clear();
}
