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
#define ASSET_GALAXY_FLEET_IMAGES 205

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

#define MULTIPLAYER_ARCHIVE "multigm.lbx"
#define ASSET_PSELECT_BG 0
#define ASSET_PSELECT_HEADER 19
#define ASSET_PSELECT_ROW 20
#define ASSET_PSELECT_FOOTER 21
#define ASSET_PSELECT_FLAGS 46

#define GALAXY_ANIM_LENGTH 8
#define GALAXY_ANIM_SPEED 120
#define GALAXY_SIDEBAR_X 579

#define PSELECT_BUFSIZE 128
#define PSELECT_ANIM_LENGTH 8
#define PSELECT_YPOS 36

const unsigned galaxySizeFactors[] = {10, 15, 20, 30, 0};
static const unsigned galaxy_fontanim[GALAXY_ANIM_LENGTH] = {
	1, 2, 3, 4, 3, 2, 1, 0
};

GalaxyView::GalaxyView(GameState *game) : _game(game), _zoom(0), _zoomX(0),
	_zoomY(0), _startTick(0), _activePlayer(-1) {
	uint8_t tpal[PALSIZE];
	const uint8_t *pal;
	int i, j, k;

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

	// Player and Antaran fleet zoom order is reversed
	for (i = 0, k = 0; i < MAX_PLAYERS + 1; i++) {
		for (j = GALAXY_ZOOM_LEVELS - 1; j >= 0; j--, k++) {
			_fleetimg[i][j] = gameAssets->getImage(GALAXY_ARCHIVE,
				ASSET_GALAXY_FLEET_IMAGES + k, tpal);
		}
	}

	for (; i < MAX_FLEET_OWNERS; i++) {
		for (j = 0; j < GALAXY_ZOOM_LEVELS; j++, k++) {
			_fleetimg[i][j] = gameAssets->getImage(GALAXY_ARCHIVE,
				ASSET_GALAXY_FLEET_IMAGES + k, tpal);
		}
	}

	_bg = gameAssets->getImage(STARBG_ARCHIVE, ASSET_STARBG, pal);

	for (i = 0, k = 0; i < NEBULA_TYPE_COUNT; i++) {
		for (j = 0; j < GALAXY_ZOOM_LEVELS; j++, k++) {
			_nebulaimg[i][j] = gameAssets->getImage(STARBG_ARCHIVE,
				ASSET_STARBG_NEBULA_IMAGES + k, tpal);
		}
	}

	try {
		initWidgets();
	} catch (...) {
		clearWidgets();
		throw;
	}
}

GalaxyView::~GalaxyView(void) {
	delete _game;
}

void GalaxyView::initWidgets(void) {
	Widget *w;
	const uint8_t *pal = _gui->palette();

	w = createWidget(249, 5, 59, 17);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickGameMenu));
	w->setClickSprite(MBUTTON_LEFT, GALAXY_ARCHIVE,
		ASSET_GALAXY_GAME_BUTTON, pal, 1);
}

int GalaxyView::transformX(int x) const {
	return 21 + 10 * (x - _zoomX) / galaxySizeFactors[_zoom];
}

int GalaxyView::transformY(int y) const {
	return 21 + 10 * (y - _zoomY) / galaxySizeFactors[_zoom];
}

int GalaxyView::transformFleetX(const Fleet *f) const {
	const Image *img;
	unsigned size;
	int x;

	switch (f->getStatus()) {
	case ShipState::InOrbit:
		img = getFleetSprite(f);
		size = f->getOrbitedStar()->size;
		x = _zoom ? 16 - 2 * _zoom : 18 - size;
		return transformX(f->getX()) + x - size - img->width() / 2;

	case ShipState::LeavingOrbit:
		img = getFleetSprite(f);
		size = f->getOrbitedStar()->size;
		x = _zoom ? 13 - 2 * _zoom : 15 - size;
		return transformX(f->getX()) - x + size - img->width() / 2;

	case ShipState::InTransit:
		return transformX(f->getX()) - 5 - (_zoom + 1) / 2;
	}

	throw std::runtime_error("Invalid fleet state");
}

int GalaxyView::transformFleetY(const Fleet *f) const {
	unsigned size;
	int y;

	switch (f->getStatus()) {
	case ShipState::InOrbit:
		size = f->getOrbitedStar()->size;
		y = _zoom ? 0 : size - 2;
		return transformY(f->getY()) + y - 14 + size;

	case ShipState::LeavingOrbit:
		size = f->getOrbitedStar()->size;
		y = _zoom ? 0 : size - 2;
		return transformY(f->getY()) + y - 15 + size;

	case ShipState::InTransit:
		return transformY(f->getY()) - 5;
	}

	throw std::runtime_error("Invalid fleet state");
}

const Image *GalaxyView::getFleetSprite(const Fleet *f) const {
	unsigned cls = f->getOwner();

	if (cls < MAX_PLAYERS) {
		cls = _game->_players[cls].color;
	}

	return (const Image*)_fleetimg[cls][_zoom];
}

void GalaxyView::selectPlayer(void) {
	unsigned i, humans;
	int last_human = -1;
	const Player *ptr;
	GuiView *view = NULL;

	for (i = 0, humans = 0; i < _game->_playerCount; i++) {
		ptr = _game->_players + i;

		if (ptr->objective == OBJECTIVE_HUMAN &&
			ptr->networkPlayerId == 0) {
			humans++;
			last_human = i;
		}
	}

	if (humans > 1) {
		try {
			view = new SelectPlayerView(_game,
				GuiMethod(*this,
				&GalaxyView::setPlayer));
			gui_stack->push(view);
		} catch (...) {
			delete view;
			throw;
		}

		return;
	} else if (humans == 1) {
		setPlayer(last_human, 0, 0);
	} else {
		throw std::runtime_error("No human players in game");
	}
}

void GalaxyView::setPlayer(int player, int a, int b) {
	if (player < 0 || player >= _game->_playerCount) {
		throw std::out_of_range("Invalid player ID");
	}

	_activePlayer = player;
}

void GalaxyView::redrawSidebar(unsigned curtick) {
	unsigned stardate, color, negcolor, x;
	Font *fnt;
	const Player *ptr;
	char buf[32], *str;

	if (_activePlayer < 0) {
		return;
	}

	// FIXME: Use strings from game data
	ptr = _game->_players + _activePlayer;
	fnt = gameFonts.getFont(FONTSIZE_SMALL);
	negcolor = (curtick - _startTick) / GALAXY_ANIM_SPEED;
	negcolor = galaxy_fontanim[negcolor % GALAXY_ANIM_LENGTH];
	negcolor += FONT_COLOR_DEFICIT1;
	stardate = _game->_gameConfig.stardate;
	sprintf(buf, "%d.%d", stardate / 10, stardate % 10);
	fnt->centerText(GALAXY_SIDEBAR_X, 29, FONT_COLOR_GALAXY_GUI, buf,
		OUTLINE_FULL);

	// Treasury and income
	sprintf(buf, "%'d BC", ptr->BC);
	color = ptr->BC < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 96, color, buf, OUTLINE_FULL);
	sprintf(buf, "%+'d BC", ptr->surplusBC);
	color = ptr->surplusBC < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 108, color, buf, OUTLINE_FULL);

	// Fleet command points
	sprintf(buf, "%+'d (%'u)", ptr->commandPoints - ptr->usedCommandPoints,
		ptr->commandPoints);
	x = GALAXY_SIDEBAR_X - fnt->textWidth(buf) / 2;
	str = strchr(buf, ' ');
	*str = '\0';
	color = FONT_COLOR_GALAXY_GUI;

	if (ptr->usedCommandPoints > (int)ptr->commandPoints) {
		color = negcolor;
	}

	x = fnt->renderText(x, 181, color, buf, OUTLINE_FULL);
	*str = ' ';
	fnt->renderText(x, 181, FONT_COLOR_GALAXY_GUI, str, OUTLINE_FULL);

	// Food surplus
	sprintf(buf, "%+'d", ptr->surplusFood);
	color = ptr->surplusFood < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 255, color, buf, OUTLINE_FULL);

	// Freighters status
	sprintf(buf, "%+'d (%'u)", ptr->surplusFreighters,
		ptr->totalFreighters);
	x = GALAXY_SIDEBAR_X - fnt->textWidth(buf) / 2;
	str = strchr(buf, ' ');
	*str = '\0';
	color = (ptr->surplusFreighters < 0) ? negcolor : FONT_COLOR_GALAXY_GUI;
	x = fnt->renderText(x, 330, color, buf, OUTLINE_FULL);
	*str = ' ';
	fnt->renderText(x, 330, FONT_COLOR_GALAXY_GUI, str, OUTLINE_FULL);

	// Research progress
	if (ptr->researchItem) {
		// FIXME: draw expected time to completion
		//sprintf(buf, "~%d turns", turns_remaining);
		//fnt->centerText(GALAXY_SIDEBAR_X, 391, FONT_COLOR_GALAXY_GUI,
		//	buf, OUTLINE_FULL);
		sprintf(buf, "%+'d RP", ptr->researchProduced);
		fnt->centerText(GALAXY_SIDEBAR_X, 403, FONT_COLOR_GALAXY_GUI,
			buf, OUTLINE_FULL);
	} else {
		fnt->centerText(GALAXY_SIDEBAR_X, 403, FONT_COLOR_GALAXY_GUI,
			"none", OUTLINE_FULL);
	}
}

void GalaxyView::open(void) {
	_startTick = 0;

	if (_activePlayer < 0) {
		selectPlayer();
	}
}

void GalaxyView::redraw(unsigned curtick) {
	unsigned i, cls, frame, bhshift = 0;
	int x, y;
	const Image *img;
	Font *fnt;
	BilistNode<Fleet> *fnode;
	unsigned font_sizes[] = {FONTSIZE_MEDIUM, FONTSIZE_SMALL,
		FONTSIZE_SMALL, FONTSIZE_SMALLER};

	if (!_startTick) {
		_startTick = curtick;
	}

	fnt = gameFonts.getFont(font_sizes[_zoom]);

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
			y += img->height();
			fnt->centerText(x, y, FONT_COLOR_STAR_NEUTRAL2,
				ptr->name, OUTLINE_FULL);
		} else if (ptr->spectralClass == SpectralClass::BlackHole) {
			img = (Image*)_bholeimg[_zoom];
			// Draw different frame for each black hole
			// using bhshift as a counter
			frame = (curtick - _startTick) / 120 + bhshift++;
			frame %= img->frameCount();
			img->draw(x - img->width() / 2, y - img->height() / 2,
				frame);
		}

		// TODO: Draw up to 3 fleets from both groups
		fnode = ptr->getOrbitingFleets();

		if (fnode && fnode->data) {
			img = getFleetSprite(fnode->data);
			img->draw(transformFleetX(fnode->data),
				transformFleetY(fnode->data));
		}

		fnode = ptr->getLeavingFleets();

		if (fnode && fnode->data) {
			img = getFleetSprite(fnode->data);
			img->draw(transformFleetX(fnode->data),
				transformFleetY(fnode->data));
		}
	}

	for (fnode = _game->getMovingFleets(); fnode; fnode = fnode->next()) {
		if (!fnode->data) {
			continue;
		}

		img = getFleetSprite(fnode->data);
		img->draw(transformFleetX(fnode->data),
			transformFleetY(fnode->data));
	}

	_gui->draw(0, 0);
	redrawSidebar(curtick);
	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void GalaxyView::clickGameMenu(int x, int y, int arg) {
	new MainMenuWindow(this, _game);
}

SelectPlayerView::SelectPlayerView(const GameState *game,
	const GuiCallback &callback) : _game(game), _callback(callback),
	_playerCount(0), _currentPlayer(-1) {

	const uint8_t *pal;
	unsigned i, flag_id;
	const Player *ptr;

	_bg = gameAssets->getImage(MULTIPLAYER_ARCHIVE, ASSET_PSELECT_BG);
	pal = _bg->palette();
	_header = gameAssets->getImage(MULTIPLAYER_ARCHIVE,
		ASSET_PSELECT_HEADER, pal);
	_row = gameAssets->getImage(MULTIPLAYER_ARCHIVE, ASSET_PSELECT_ROW,
		pal);
	_footer = gameAssets->getImage(MULTIPLAYER_ARCHIVE,
		ASSET_PSELECT_FOOTER, pal);

	for (i = 0; i < _game->_playerCount; i++) {
		ptr = _game->_players + i;

		if (ptr->objective != OBJECTIVE_HUMAN ||
			ptr->networkPlayerId != 0) {
			continue;
		}

		if (ptr->picture >= RACE_COUNT) {
			throw std::out_of_range("Player has invalid race ID");
		}

		if (ptr->color >= PLAYER_COUNT) {
			throw std::out_of_range("Player has invalid color ID");
		}

		flag_id = ptr->picture * PLAYER_COUNT + ptr->color;
		_playerFlags[_playerCount] = gameAssets->getImage(
			MULTIPLAYER_ARCHIVE, ASSET_PSELECT_FLAGS + flag_id,
			pal);
		_humans[_playerCount++] = i;
	}

	try {
		initWidgets();
	} catch (...) {
		clearWidgets();
		throw;
	}
}

void SelectPlayerView::initWidgets(void) {
	Widget *w;
	unsigned i, x, y, h;

	h = _row->height();
	x = (SCREEN_WIDTH - _header->width()) / 2;
	y = PSELECT_YPOS;

	for (i = 0; i < _playerCount; i++) {
		if (_game->_players[_humans[i]].playerDoneFlags) {
			continue;
		}

		// Player flag
		w = createWidget(x + 37, y + 63 + i * h, 34, 36);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &SelectPlayerView::clickPlayer,
			_humans[i]));
		w->setMouseOverCallback(
			GuiMethod(*this, &SelectPlayerView::highlightPlayer,
			i));
		w->setMouseOutCallback(
			GuiMethod(*this, &SelectPlayerView::highlightPlayer,
			-1));

		// Player name
		w = createWidget(x + 104, y + 66 + i * h, 335, 30);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &SelectPlayerView::clickPlayer,
			_humans[i]));
		w->setMouseOverCallback(
			GuiMethod(*this, &SelectPlayerView::highlightPlayer,
			i));
		w->setMouseOutCallback(
			GuiMethod(*this, &SelectPlayerView::highlightPlayer,
			-1));
	}
}

void SelectPlayerView::redraw(unsigned curtick) {
	unsigned i, x, y, h, color;
	const Player *ptr;
	Font *fnt;
	char buf[PSELECT_BUFSIZE];
	unsigned fontanim[PSELECT_ANIM_LENGTH] = {3, 4, 3, 2, 1, 0, 1, 2};
	unsigned color_list[] = {
		FONT_COLOR_PLAYER_RED1, FONT_COLOR_PLAYER_YELLOW1,
		FONT_COLOR_PLAYER_GREEN1, FONT_COLOR_PLAYER_SILVER1,
		FONT_COLOR_PLAYER_BLUE1, FONT_COLOR_PLAYER_BROWN1,
		FONT_COLOR_PLAYER_PURPLE1, FONT_COLOR_PLAYER_ORANGE1
	};

	if (!_animStart) {
		_animStart = curtick;
	}

	fnt = gameFonts.getFont(FONTSIZE_BIG);
	h = _row->height();
	x = (SCREEN_WIDTH - _header->width()) / 2;
	y = PSELECT_YPOS;

	_bg->draw(0, 0);
	_header->draw(x, y);
	y += _header->height();

	for (i = 0; i < _playerCount - 1; i++) {
		_row->draw(x, y + i * h);
	}

	_footer->draw(x, y + i * h);

	for (i = 0; i < _playerCount; i++) {
		ptr = _game->_players + _humans[i];
		// FIXME: Use strings from game data
		// FIXME: Find and use real player score
		snprintf(buf, PSELECT_BUFSIZE, "%s, %s, Score %d",
			ptr->name, ptr->race, 0);
		buf[PSELECT_BUFSIZE - 1] = '\0';

		if (_game->_players[_humans[i]].playerDoneFlags) {
			color = 1;
		} else if (int(i) == _currentPlayer) {
			color = (curtick - _animStart) / 30;
			color = fontanim[color % PSELECT_ANIM_LENGTH];
		} else {
			color = 3;
		}

		color += color_list[ptr->color];
		fnt->renderText(x + 116, y + i * h - 9, color, buf);

		if (!_game->_players[_humans[i]].playerDoneFlags) {
			_playerFlags[i]->draw(x + 40, y + i * h - 16);
		}
	}
}

void SelectPlayerView::highlightPlayer(int x, int y, int arg) {
	_currentPlayer = arg;
	_animStart = 0;
}

void SelectPlayerView::clickPlayer(int x, int y, int arg) {
	_callback(arg, 0);
	exitView();
}

MainMenuWindow::MainMenuWindow(GuiView *parent, GameState *game) :
	GuiWindow(parent, WINDOW_MODAL), _game(game) {

	ImageAsset img;
	const uint8_t *pal;

	img = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = img->palette();
	_bg = gameAssets->getImage(GAMEMENU_ARCHIVE, ASSET_GALAXY_GUI, pal);

	_x = 144;
	_y = 25;
	_width = _bg->width();
	_height = _bg->height();

	try {
		initWidgets(pal);
	} catch (...) {
		clearWidgets();
		throw;
	}
}

MainMenuWindow::~MainMenuWindow(void) {

}

void MainMenuWindow::initWidgets(const uint8_t *pal) {
	Widget *w;

	// Save Game button
	w = createWidget(40, 43, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickSave));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_SAVE,
		pal, 1);

	// Load Game button
	w = createWidget(147, 43, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickLoad));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_LOAD,
		pal, 1);

	// New Game button
	w = createWidget(40, 88, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickNew));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_NEW,
		pal, 1);

	// Quit Game button
	w = createWidget(147, 88, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickQuit));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_QUIT,
		pal, 1);

	// Settings button
	w = createWidget(40, 307, 91, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickSettings));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
		ASSET_GAMEMENU_SETTINGS, pal, 1);

	// Return button
	w = createWidget(151, 307, 91, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod<GuiWindow>(*this, &MainMenuWindow::close));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
		ASSET_GAMEMENU_RETURN, pal, 1);
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
