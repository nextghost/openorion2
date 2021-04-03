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
#include "mainmenu.h"

#define ASSET_MENU_BACKGROUND 2

MainMenuView::MainMenuView(void) : _background(NULL) {
	ActiveZone *zone;

	_background = gameAssets->getImage("mainmenu.lbx",
		ASSET_MENU_BACKGROUND);

	zone = new ActiveZone(415, 172, 153, 23);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickContinue);
	addZone(zone);

	zone = new ActiveZone(415, 195, 153, 22);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickLoad);
	addZone(zone);

	zone = new ActiveZone(415, 217, 153, 23);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickNew);
	addZone(zone);

	zone = new ActiveZone(415, 240, 153, 22);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickMultiplayer);
	addZone(zone);

	zone = new ActiveZone(415, 262, 153, 23);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickScoreboard);
	addZone(zone);

	zone = new ActiveZone(415, 285, 153, 22);
	zone->onMouseUp = GuiMethod(*this, &MainMenuView::clickQuit);
	addZone(zone);
}

MainMenuView::~MainMenuView(void) {
	gameAssets->freeImage(_background);
}

void MainMenuView::redraw(unsigned curtick) {
	_background->draw(0, 0);
}

void MainMenuView::clickContinue(int x, int y, int arg) {
	printf("Continue clicked\n");
}

void MainMenuView::clickLoad(int x, int y, int arg) {
	printf("Load Game clicked\n");
}

void MainMenuView::clickNew(int x, int y, int arg) {
	printf("New Game clicked\n");
}

void MainMenuView::clickMultiplayer(int x, int y, int arg) {
	printf("Multiplayer clicked\n");
}

void MainMenuView::clickScoreboard(int x, int y, int arg) {
	printf("Hall of Fame clicked\n");
}

void MainMenuView::clickQuit(int x, int y, int arg) {
	printf("Quit clicked\n");
}
