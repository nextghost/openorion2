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
#include "mainmenu.h"
#include "galaxy.h"

#define GALAXY_ARCHIVE "buffer0.lbx"
#define ASSET_GALAXY_GUI 0
#define ASSET_GALAXY_GAME_BUTTON 1
#define ASSET_GALAXY_STAR_IMAGES 148
#define ASSET_GALAXY_BHOLE_IMAGES 184

#define STARBG_ARCHIVE "starbg.lbx"
#define ASSET_STARBG 3
#define ASSET_STARBG_NEBULA_IMAGES 6

#define GAMEMENU_ARCHIVE "game.lbx"
#define ASSET_GAMEMENU_BG 0
#define ASSET_GAMEMENU_SAVE 1
#define ASSET_GAMEMENU_LOAD 2
#define ASSET_GAMEMENU_NEW 3
#define ASSET_GAMEMENU_QUIT 4
#define ASSET_GAMEMENU_SETTINGS 5
#define ASSET_GAMEMENU_RETURN 6

const unsigned galaxySizeFactors[] = {10, 15, 20, 30, 0};

GalaxyView::GalaxyView(GameState *game) : _game(game), _zoom(0), _zoomX(0),
	_zoomY(0) {
	uint8_t tpal[PALSIZE];
	const uint8_t *pal;
	int i, j, k;
	Widget *w;

	for (i = 0; galaxySizeFactors[i]; i++) {
		if (galaxySizeFactors[i] == game->_galaxy.sizeFactor) {
			_zoom = i;
			break;
		}
	}

	if (!galaxySizeFactors[i]) {
		throw std::runtime_error("Invalid galaxy size");
	}

	_gui = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = _gui->palette();
	// convert to transparent palette
	memcpy(tpal, pal, PALSIZE);
	memset(tpal, 0, 4);

	for (i = 0, k = 0; i < STAR_TYPE_COUNT; i++) {
		for (j = 0; j < GALAXY_STAR_SIZES; j++, k++) {
			_starimg[i][j] = gameAssets->getImage(GALAXY_ARCHIVE,
				ASSET_GALAXY_STAR_IMAGES + k, tpal);
		}
	}

	for (i = 0; i < GALAXY_ZOOM_LEVELS; i++) {
		_bholeimg[i] = gameAssets->getImage(GALAXY_ARCHIVE,
			ASSET_GALAXY_BHOLE_IMAGES + i, tpal);
	}

	_bg = gameAssets->getImage(STARBG_ARCHIVE, ASSET_STARBG, pal);

	for (i = 0, k = 0; i < NEBULA_TYPE_COUNT; i++) {
		for (j = 0; j < GALAXY_ZOOM_LEVELS; j++, k++) {
			_nebulaimg[i][j] = gameAssets->getImage(STARBG_ARCHIVE,
				ASSET_STARBG_NEBULA_IMAGES + k, tpal);
		}
	}

	try {
		w = new Widget(249, 5, 59, 17);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &GalaxyView::clickGameMenu));
		w->setClickSprite(MBUTTON_LEFT, GALAXY_ARCHIVE,
			ASSET_GALAXY_GAME_BUTTON, pal, 1);
		addWidget(w);
		w = NULL;
	} catch (...) {
		delete w;
		throw;
	}
}

GalaxyView::~GalaxyView(void) {
	delete _game;
}

int GalaxyView::transformX(int x) const {
	return 21 + 10 * (x - _zoomX) / galaxySizeFactors[_zoom];
}

int GalaxyView::transformY(int y) const {
	return 21 + 10 * (y - _zoomY) / galaxySizeFactors[_zoom];
}

void GalaxyView::open(void) {
	_startTick = 0;
}

void GalaxyView::redraw(unsigned curtick) {
	unsigned i, cls, frame, bhshift = 0;
	int x, y;
	Image *img;
	Font *fnt;
	unsigned font_sizes[] = {3, 2, 2, 1};
	uint8_t palette[] = {0, 0, 0, 0, 255, 0, 12, 0, 255, 108, 108, 116,
		255, 0, 12, 0};

	if (!_startTick) {
		_startTick = curtick;
	}

	fnt = gameFonts.getFont(font_sizes[_zoom]);
	fnt->setPalette(palette, 4);

	clearScreen();
	_bg->draw(0, 0);

	for (i = 0; i < _game->_galaxy.nebulaCount; i++) {
		Nebula *ptr = _game->_galaxy.nebulas + i;

		x = transformX(ptr->x);
		y = transformY(ptr->y);
		_nebulaimg[ptr->type][_zoom]->draw(x, y);
	}

	// Draw wormholes
	for (i = 0; i < _game->_starSystemCount; i++) {
		Star *s1, *s2;

		s1 = _game->_starSystems + i;

		if (s1->wormhole < (int)i) {
			continue;
		}

		s2 = _game->_starSystems + s1->wormhole;
		drawLine(transformX(s1->x), transformY(s1->y),
			transformX(s2->x), transformY(s2->y), 36, 36, 40);
	}

	// Draw stars and black holes
	for (i = 0; i < _game->_starSystemCount; i++) {
		Star *ptr = _game->_starSystems + i;

		x = transformX(ptr->x);
		y = transformY(ptr->y);

		if (ptr->spectralClass < SpectralClass::BlackHole) {
			cls = ptr->spectralClass;
			img = (Image*)_starimg[cls][_zoom + ptr->size];
			y -= img->height() / 2;
			img->draw(x - img->width() / 2, y);
			x -= fnt->textWidth(ptr->name) / 2;
			y += img->height();
			fnt->renderText(x, y, ptr->name);
		} else if (ptr->spectralClass == SpectralClass::BlackHole) {
			img = (Image*)_bholeimg[_zoom];
			// Draw different frame for each black hole
			// using bhshift as a counter
			frame = (curtick - _startTick) / 120 + bhshift++;
			frame %= img->frameCount();
			img->draw(x - img->width() / 2, y - img->height() / 2,
				frame);
		}
	}

	_gui->draw(0, 0);
	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void GalaxyView::clickGameMenu(int x, int y, int arg) {
	new MainMenuWindow(this, _game);
}

MainMenuWindow::MainMenuWindow(GuiView *parent, GameState *game) :
	GuiWindow(parent, WINDOW_MODAL), _game(game) {

	ImageAsset img;
	const uint8_t *pal;
	Widget *w;

	img = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = img->palette();
	_bg = gameAssets->getImage(GAMEMENU_ARCHIVE, ASSET_GALAXY_GUI, pal);

	_x = 144;
	_y = 25;
	_width = _bg->width();
	_height = _bg->height();

	try {
		w = new Widget(40, 43, 91, 28);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickSave));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_SAVE, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(40, 43, 91, 28);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickSave));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_SAVE, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(147, 43, 91, 28);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickLoad));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_LOAD, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(40, 88, 91, 28);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickNew));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_NEW, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(147, 88, 91, 28);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickQuit));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_QUIT, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(40, 307, 91, 27);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &MainMenuWindow::clickSettings));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_SETTINGS, pal, 1);
		addWidget(w);
		w = NULL;

		w = new Widget(151, 307, 91, 27);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod<GuiWindow>(*this, &MainMenuWindow::close));
		w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
			ASSET_GAMEMENU_RETURN, pal, 1);
		addWidget(w);
		w = NULL;
	} catch (...) {
		delete w;
		throw;
	}
}

MainMenuWindow::~MainMenuWindow(void) {

}

void MainMenuWindow::redraw(unsigned curtick) {
	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);
}

void MainMenuWindow::clickNew(int x, int y, int arg) {
	new MessageBoxWindow(_parent, "New Game not implemented yet");
}

void MainMenuWindow::clickSave(int x, int y, int arg) {
	new MessageBoxWindow(_parent, "Save Game not implemented yet");
}

void MainMenuWindow::clickLoad(int x, int y, int arg) {
	new LoadGameWindow(_parent, 0);
}

void MainMenuWindow::clickQuit(int x, int y, int arg) {
	_parent->close();
	gui_stack->clear();
}

void MainMenuWindow::clickSettings(int x, int y, int arg) {
	new MessageBoxWindow(_parent, "Settings not implemented yet");
}
