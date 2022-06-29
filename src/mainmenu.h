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

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include <time.h>
#include "lbx.h"
#include "gui.h"
#include "gamestate.h"

#define SAVEGAME_SLOTS 10

class MainMenuView : public GuiView {
private:
	ImageAsset _background;

	void initWidgets(void);

public:
	MainMenuView(void);
	~MainMenuView(void);

	void redraw(unsigned curtick);

	void showHelp(int x, int y, int arg);
	void clickContinue(int x, int y, int arg);
	void clickLoad(int x, int y, int arg);
	void clickNew(int x, int y, int arg);
	void clickMultiplayer(int x, int y, int arg);
	void clickScoreboard(int x, int y, int arg);
	void clickQuit(int x, int y, int arg);
};

struct SaveGameInfo {
	char *filename;
	time_t mtime;
	GameConfig header;

	SaveGameInfo(void);
	~SaveGameInfo(void);
};

class LoadGameWindow : public GuiWindow {
private:
	ImageAsset _bg, _singleIcon, _hotseatIcon, _networkIcon, _modemIcon;
	int _quickload, _selected;
	SaveGameInfo *_saveFiles;

	void initWidgets(const uint8_t *palette);

protected:
	void drawSlot(unsigned slot, Font *fnt, Font *smallfnt);

public:
	LoadGameWindow(GuiView *parent, int quickload);
	~LoadGameWindow(void);

	void redraw(unsigned curtick);

	void selectSlot(int x, int y, int slot);
	void handleLoad(int x = 0, int y = 0, int arg = 0);
};

#endif
