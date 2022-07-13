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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "lbx.h"
#include "screen.h"
#include "system.h"
#include "guimisc.h"
#include "galaxy.h"
#include "lang.h"
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

#define GAMEMENU_ARCHIVE "game.lbx"
#define ASSET_LOAD_BACKGROUND 20
#define ASSET_LOAD_LOADBUTTON 21
#define ASSET_LOAD_CANCEL 22
#define ASSET_LOAD_SINGLE 23
#define ASSET_LOAD_HOTSEAT 24
#define ASSET_LOAD_NETWORK 25
#define ASSET_LOAD_MODEM 26

SaveGameInfo::SaveGameInfo(void) : filename(NULL), mtime(0) {

}

SaveGameInfo::~SaveGameInfo(void) {
	delete[] filename;
}

SaveGameInfo *findSavedGames(void) {
	unsigned slot;
	int tmp;
	char c;
	DIR *dptr;
	struct dirent *entry;
	struct stat stbuf;
	char *fname, *path = configPath(NULL);
	SaveGameInfo *ret;
	File fr;

	dptr = opendir(path);
	delete[] path;

	if (!dptr) {
		throw std::runtime_error("Could not open savegame directory");
	}

	try {
		ret = new SaveGameInfo[SAVEGAME_SLOTS];
	} catch (...) {
		closedir(dptr);
		throw;
	}

	while ((entry = readdir(dptr))) {
		fname = strlower(entry->d_name);
		tmp = sscanf(fname, "save%d.gam%c", &slot, &c);
		delete[] fname;

		if (tmp != 1 || slot < 1 || slot > SAVEGAME_SLOTS) {
			continue;
		}

		--slot;
		path = configPath(entry->d_name);
		tmp = stat(path, &stbuf);
		fr.open(path);
		delete[] path;

		if (tmp || !S_ISREG(stbuf.st_mode) || !fr.isOpen()) {
			continue;
		}

		if (ret[slot].filename) {
			fprintf(stderr, "Warning: duplicate savegame file in slot %u\n", slot);
			continue;
		}

		try {
			ret[slot].header.load(fr);
		} catch (...) {
			continue;
		}

		fr.close();
		ret[slot].filename = copystr(entry->d_name);
		ret[slot].mtime = stbuf.st_mtime;
	}

	closedir(dptr);
	return ret;
}

void loadGame(const char *filename) {
	GameState* game = NULL;
	GuiView *view = NULL;
	char *path = NULL;

	try {
		path = configPath(filename);
		game = new GameState;
		game->load(path);
		game->dump();
		view = new GalaxyView(game);
		game = NULL;
		gui_stack->push(view);
	} catch (std::exception &e) {
		delete[] path;
		delete game;
		delete view;
		throw;
	}

	delete[] path;
}

MainMenuView::MainMenuView(void) {
	_background = gameAssets->getImage(MENU_ARCHIVE, ASSET_MENU_BACKGROUND);

	initWidgets();
}

MainMenuView::~MainMenuView(void) {

}

void MainMenuView::initWidgets(void) {
	Widget *w;
	SaveGameInfo *saveFiles = NULL;
	const uint8_t *pal;
	int savecount = 0, autosave = 0;

	try {
		saveFiles = findSavedGames();
		autosave = saveFiles[SAVEGAME_SLOTS - 1].filename != NULL;

		for (int i = 0; i < SAVEGAME_SLOTS; i++) {
			if (saveFiles[i].filename) {
				savecount++;
			}
		}
	} catch (...) {
		// ignore errors
	}

	delete[] saveFiles;
	pal = _background->palette();

	// Continue button
	w = createWidget(415, 172, 153, 23);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_CONTINUE));

	if (autosave) {
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickContinue));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_CONTINUE_ON,
			pal, 0);
	} else {
		w->setIdleSprite(MENU_ARCHIVE, ASSET_MENU_CONTINUE_OFF, pal,
			0);
	}

	// Load Game button
	w = createWidget(415, 195, 153, 22);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_LOAD));

	if (savecount) {
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuView::clickLoad));
		w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_LOAD_ON, pal,
			0);
	} else {
		w->setIdleSprite(MENU_ARCHIVE, ASSET_MENU_LOAD_OFF, pal, 0);
	}

	// New Game button
	w = createWidget(415, 217, 153, 23);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuView::clickNew));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_NEWGAME));
	w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_NEWGAME, pal, 0);

	// Multiplayer button
	w = createWidget(415, 240, 153, 22);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuView::clickMultiplayer));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_MULTIPLAYER));
	w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_MULTIPLAYER, pal, 0);

	// Hall of Fame button
	w = createWidget(415, 262, 153, 23);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuView::clickScoreboard));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_SCORES));
	w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_SCORES, pal, 0);

	// Quit Game button
	w = createWidget(415, 285, 153, 22);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuView::clickQuit));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuView::showHelp,
		HELP_MAINMENU_QUIT));
	w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_QUIT, pal, 0);
}

void MainMenuView::redraw(unsigned curtick) {
	_background->draw(0, 0);
	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void MainMenuView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _background->palette());
}

void MainMenuView::clickContinue(int x, int y, int arg) {
	SaveGameInfo *saveFiles = NULL;
	int slot = SAVEGAME_SLOTS - 1;

	try {
		saveFiles = findSavedGames();
	} catch (...) {
		new ErrorWindow(this, "Cannot read saved games");
		return;
	}

	if (!saveFiles[slot].filename) {
		new ErrorWindow(this, "GAME10.SAV does not exist");
		delete[] saveFiles;
		return;
	}

	try {
		loadGame(saveFiles[slot].filename);
	} catch (std::exception &e) {
		char buf[64];

		fprintf(stderr, "Error loading %s: %s\n",
			saveFiles[slot].filename, e.what());
		sprintf(buf, "Cannot load %s", saveFiles[slot].filename);
		new ErrorWindow(this, buf);
		delete[] saveFiles;
		return;
	}

	delete[] saveFiles;
	exitView();
}

void MainMenuView::clickLoad(int x, int y, int arg) {
	new LoadGameWindow(this, 0);
}

void MainMenuView::clickNew(int x, int y, int arg) {
	new MessageBoxWindow(this, "New Game not implemented yet");
}

void MainMenuView::clickMultiplayer(int x, int y, int arg) {
	new MessageBoxWindow(this, "Multiplayer not implemented yet");
}

void MainMenuView::clickScoreboard(int x, int y, int arg) {
	new MessageBoxWindow(this, "Hall of Fame not implemented yet");
}

void MainMenuView::clickQuit(int x, int y, int arg) {
	gui_stack->clear();
}

LoadGameWindow::LoadGameWindow(GuiView *parent, int quickload) :
	GuiWindow(parent, WINDOW_MODAL), _quickload(quickload), _selected(-1),
	_saveFiles(NULL) {

	ImageAsset tmp;
	const uint8_t *pal;

	tmp = gameAssets->getImage(MENU_ARCHIVE, ASSET_MENU_BACKGROUND);
	pal = tmp->palette();
	_bg = gameAssets->getImage(GAMEMENU_ARCHIVE, ASSET_LOAD_BACKGROUND,
		pal);
	_singleIcon = gameAssets->getImage(GAMEMENU_ARCHIVE, ASSET_LOAD_SINGLE,
		pal);
	_hotseatIcon = gameAssets->getImage(GAMEMENU_ARCHIVE,
		ASSET_LOAD_HOTSEAT, pal);
	_networkIcon = gameAssets->getImage(GAMEMENU_ARCHIVE,
		ASSET_LOAD_NETWORK, pal);
	_modemIcon = gameAssets->getImage(GAMEMENU_ARCHIVE, ASSET_LOAD_MODEM,
		pal);

	_width = _bg->width();
	_height = _bg->height();
	_x = (SCREEN_WIDTH - _width) / 2;
	_y = (SCREEN_HEIGHT - _height) / 2;

	initWidgets(pal);
	_saveFiles = findSavedGames();
}

LoadGameWindow::~LoadGameWindow(void) {
	delete[] _saveFiles;
}

void LoadGameWindow::initWidgets(const uint8_t *pal) {
	Widget *w;

	// Load button
	w = createWidget(37, 337, 68, 22);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &LoadGameWindow::handleLoad));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
		ASSET_LOAD_LOADBUTTON, pal, 1);

	// Cancel button
	w = createWidget(171, 338, 68, 22);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod<GuiWindow>(*this, &LoadGameWindow::close));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_LOAD_CANCEL,
		pal, 1);

	// Savegame slots
	for (int i = 0; i < SAVEGAME_SLOTS; i++) {
		w = createWidget(22, 22 + 31 * i, 232, 27);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &LoadGameWindow::selectSlot, i));
	}
}

void LoadGameWindow::drawSlot(unsigned slot, Font *fnt, Font *smallfnt) {
	int y = _y + 24 + 31 * slot;
	unsigned stardate, color = FONT_COLOR_SAVEGAME;
	struct tm *ltime;
	char buf[64];

	if (int(slot) == _selected) {
		color = FONT_COLOR_SAVEGAME_SEL;
	}

	if (!_saveFiles[slot].filename) {
		// FIXME: Use strings from game data
		fnt->renderText(_x + 32, y, color, "... empty slot ...");
		return;
	}

	if (slot == SAVEGAME_SLOTS - 1) {
		fnt->renderText(_x + 32, y, color, "(Auto Save)");
	} else {
		fnt->renderText(_x + 32, y, color,
			_saveFiles[slot].header.saveGameName);
	}

	stardate = _saveFiles[slot].header.stardate;
	sprintf(buf, "Stardate: %u.%u", stardate / 10, stardate % 10);
	smallfnt->renderText(_x + 32, y + 14, color, buf);
	ltime = localtime(&_saveFiles[slot].mtime);
	strftime(buf, 64, "%x   %H:%M", ltime);
	smallfnt->renderText(_x + 122, y + 14, color, buf);

	switch (_saveFiles[slot].header.multiplayer) {
	case MultiplayerType::Single:
		_singleIcon->draw(_x + 206, y + 12);
		break;

	case MultiplayerType::Hotseat:
		_hotseatIcon->draw(_x + 206, y + 12);
		break;

	case MultiplayerType::Network:
		_networkIcon->draw(_x + 206, y + 12);
		break;
	}
}

void LoadGameWindow::redraw(unsigned curtick) {
	Font *fnt, *smallfnt;
	int i;

	fnt = gameFonts->getFont(FONTSIZE_SMALL);
	smallfnt = gameFonts->getFont(FONTSIZE_SMALLER);

	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);

	for (i = 0; i < SAVEGAME_SLOTS; i++) {
		drawSlot(i, fnt, smallfnt);
	}
}

void LoadGameWindow::selectSlot(int x, int y, int slot) {
	_selected = slot;

	if (_quickload) {
		handleLoad();
	}
}

void LoadGameWindow::handleLoad(int x, int y, int arg) {
	char buf[64];

	if (_selected < 0) {
		return;
	} else if (!_saveFiles[_selected].filename) {
		sprintf(buf, "SAVE%d.GAM does not exist", _selected + 1);
		new ErrorWindow(_parent, buf);
		return;
	}

	try {
		loadGame(_saveFiles[_selected].filename);
		_parent->exitView();
	} catch (std::exception &e) {
		fprintf(stderr, "Error loading %s: %s\n",
			_saveFiles[_selected].filename, e.what());
		sprintf(buf, "Cannot load %s", _saveFiles[_selected].filename);
		new ErrorWindow(_parent, buf);
		return;
	}
}
