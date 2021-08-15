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

MainMenuView::MainMenuView(void) : _background() {
	Widget *w = NULL;
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
	_background = gameAssets->getImage(MENU_ARCHIVE, ASSET_MENU_BACKGROUND);
	pal = _background->palette();

	try {
		w = new Widget(415, 172, 153, 23);

		if (autosave) {
			w->setMouseUpCallback(MBUTTON_LEFT,
				GuiMethod(*this, &MainMenuView::clickContinue));
			w->setMouseOverSprite(MENU_ARCHIVE,
				ASSET_MENU_CONTINUE_ON, pal, 0);
		} else {
			w->setIdleSprite(MENU_ARCHIVE, ASSET_MENU_CONTINUE_OFF,
				pal, 0);
		}

		addWidget(w);
		w = NULL;

		w = new Widget(415, 195, 153, 22);

		if (savecount) {
			w->setMouseUpCallback(MBUTTON_LEFT,
				GuiMethod(*this, &MainMenuView::clickLoad));
			w->setMouseOverSprite(MENU_ARCHIVE, ASSET_MENU_LOAD_ON,
				pal, 0);
		} else {
			w->setIdleSprite(MENU_ARCHIVE, ASSET_MENU_LOAD_OFF,
				pal, 0);
		}

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
	} catch (...) {
		char buf[64];
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
	MessageBoxWindow *w;

	w = new MessageBoxWindow(this, "New Game not implemented yet");
}

void MainMenuView::clickMultiplayer(int x, int y, int arg) {
	MessageBoxWindow *w;

	w = new MessageBoxWindow(this, "Multiplayer not implemented yet");
}

void MainMenuView::clickScoreboard(int x, int y, int arg) {
	MessageBoxWindow *w;

	w = new MessageBoxWindow(this, "Hall of Fame not implemented yet");
}

void MainMenuView::clickQuit(int x, int y, int arg) {
	gui_stack->clear();
}

LoadGameWindow::LoadGameWindow(GuiView *parent, int quickload) :
	GuiWindow(parent, WINDOW_MODAL), _quickload(quickload), _selected(-1),
	_saveFiles(NULL) {

	Widget *w = NULL;
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

	_saveFiles = findSavedGames();

	try {
		w = new Widget(37, 337, 68, 22);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &LoadGameWindow::handleLoad));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_LOAD_LOADBUTTON, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(171, 338, 68, 22);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod<GuiWindow>(*this, &LoadGameWindow::close));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_LOAD_CANCEL, pal, 1);
		addWidget(w);
		w = NULL;

		for (int i = 0; i < SAVEGAME_SLOTS; i++) {
			w = new Widget(22, 22 + 31 * i, 232, 27);
			w->setMouseUpCallback(MBUTTON_LEFT,
				GuiMethod(*this, &LoadGameWindow::selectSlot,
				i));
			addWidget(w);
			w = NULL;
		}
	} catch (...) {
		delete[] _saveFiles;
		delete w;
		clearWidgets();
		throw;
	}
}

LoadGameWindow::~LoadGameWindow(void) {
	delete[] _saveFiles;
}

void LoadGameWindow::drawSlot(unsigned slot, Font *fnt, Font *smallfnt) {
	int y = _y + 24 + 31 * slot;
	unsigned stardate;
	struct tm *ltime;
	char buf[64];

	if (!_saveFiles[slot].filename) {
		// FIXME: Use strings from game data
		fnt->renderText(_x + 32, y, "... empty slot ...");
		return;
	}

	if (slot == SAVEGAME_SLOTS - 1) {
		fnt->renderText(_x + 32, y, "(Auto Save)");
	} else {
		fnt->renderText(_x + 32, y,
			_saveFiles[slot].header.saveGameName);
	}

	stardate = _saveFiles[slot].header.stardate;
	sprintf(buf, "Stardate: %u.%u", stardate / 10, stardate % 10);
	smallfnt->renderText(_x + 32, y + 14, buf);
	ltime = localtime(&_saveFiles[slot].mtime);
	strftime(buf, 64, "%x   %H:%M", ltime);
	smallfnt->renderText(_x + 122, y + 14, buf);

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
	char buf[64];
	uint8_t palette[] = {0, 0, 0, 0, 255, 76, 40, 0, 255, 200, 100, 8};
	uint8_t palette2[] = {0, 0, 0, 0, 255, 152, 76, 12, 255, 252, 136, 0};

	fnt = gameFonts.getFont(2);
	fnt->setPalette(palette, 3);
	smallfnt = gameFonts.getFont(1);
	smallfnt->setPalette(palette, 3);

	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);

	for (i = 0; i < SAVEGAME_SLOTS; i++) {
		if (i == _selected) {
			continue;
		}

		drawSlot(i, fnt, smallfnt);
	}

	if (_selected >= 0) {
		fnt->setPalette(palette2, 3);
		smallfnt->setPalette(palette2, 3);
		drawSlot(_selected, fnt, smallfnt);
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
		sprintf(buf, "Cannot load %s", _saveFiles[_selected].filename);
		new ErrorWindow(_parent, buf);
		return;
	}
}
