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
#include "lang.h"
#include "ships.h"
#include "galaxy.h"

#define STARSEL_FRAMECOUNT 6

#define GALAXY_ARCHIVE "buffer0.lbx"
#define ASSET_GALAXY_GUI 0
#define ASSET_GALAXY_GAME_BUTTON 1
#define ASSET_GALAXY_TURN_BUTTON 2
#define ASSET_GALAXY_COLONIES_BUTTON 3
#define ASSET_GALAXY_PLANETS_BUTTON 4
#define ASSET_GALAXY_FLEETS_BUTTON 5
#define ASSET_GALAXY_ZOOMIN_BUTTON 6
#define ASSET_GALAXY_ZOOMOUT_BUTTON 7
#define ASSET_GALAXY_LEADERS_BUTTON 8
#define ASSET_GALAXY_RACES_BUTTON 9
#define ASSET_GALAXY_INFO_BUTTON 10
#define ASSET_GALAXY_STAR_IMAGES 148
#define ASSET_GALAXY_BHOLE_IMAGES 184
#define ASSET_GALAXY_FLEET_IMAGES 205
#define ASSET_GALAXY_TURN_DONE 265

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

#define PLANET_ARCHIVE "plntsum.lbx"
#define ASSET_PLANETLIST_BG 0
#define ASSET_PLANETLIST_RETURN_BUTTON 14

#define STRING_PLANET_LIST_CLIMATE_TOXIC 538
#define STRING_PLANET_LIST_CLIMATE_RADIATED 718
#define STRING_PLANET_LIST_CLIMATE_BARREN 719
#define STRING_PLANET_LIST_CLIMATE_DESERT 720
#define STRING_PLANET_LIST_CLIMATE_TUNDRA 721
#define STRING_PLANET_LIST_CLIMATE_OCEAN 398
#define STRING_PLANET_LIST_CLIMATE_SWAMP 500
#define STRING_PLANET_LIST_CLIMATE_ARID 183
#define STRING_PLANET_LIST_CLIMATE_TERRAN 722
#define STRING_PLANET_LIST_CLIMATE_GAIA 302

#define STRING_PLANET_LIST_GRAVITY_LOW 686
#define STRING_PLANET_LIST_GRAVITY_NORMAL 687
#define STRING_PLANET_LIST_GRAVITY_HEAVY 688

#define STRING_PLANET_LIST_MINERALS_ULTRA_POOR 683
#define STRING_PLANET_LIST_MINERALS_POOR 422
#define STRING_PLANET_LIST_MINERALS_ABUNDANT 684
#define STRING_PLANET_LIST_MINERALS_RICH 449
#define STRING_PLANET_LIST_MINERALS_ULTRA_RICH 685

#define STRING_PLANET_LIST_SIZE_TINY 682
#define STRING_PLANET_LIST_SIZE_SMALL 479
#define STRING_PLANET_LIST_SIZE_MEDIUM 370
#define STRING_PLANET_LIST_SIZE_LARGE 359
#define STRING_PLANET_LIST_SIZE_HUGE 322

static const uint8_t minimapStarSelColors[STARSEL_FRAMECOUNT * 3] = {
	RGB(0x043804), RGB(0x004c00), RGB(0x006000), RGB(0x087008),
	RGB(0x2c8024), RGB(0x489038)
};

const unsigned galaxySizeFactors[] = {10, 15, 20, 30, 0};
static const unsigned galaxy_fontanim[GALAXY_ANIM_LENGTH] = {
	1, 2, 3, 4, 3, 2, 1, 0
};
const char *romanNumbers[5] = { "I", "II", "III", "IV", "V"};
const int planetClimateMap[10] = {
	STRING_PLANET_LIST_CLIMATE_TOXIC,
	STRING_PLANET_LIST_CLIMATE_RADIATED,
	STRING_PLANET_LIST_CLIMATE_BARREN,
	STRING_PLANET_LIST_CLIMATE_DESERT,
	STRING_PLANET_LIST_CLIMATE_TUNDRA,
	STRING_PLANET_LIST_CLIMATE_OCEAN,
	STRING_PLANET_LIST_CLIMATE_SWAMP,
	STRING_PLANET_LIST_CLIMATE_ARID,
	STRING_PLANET_LIST_CLIMATE_TERRAN,
	STRING_PLANET_LIST_CLIMATE_GAIA
};
const int planetGravityMap[3] = {
	STRING_PLANET_LIST_GRAVITY_LOW,
	STRING_PLANET_LIST_GRAVITY_NORMAL,
	STRING_PLANET_LIST_GRAVITY_HEAVY
};
const int mineralsMap[5] = {
	STRING_PLANET_LIST_MINERALS_ULTRA_POOR,
	STRING_PLANET_LIST_MINERALS_POOR,
	STRING_PLANET_LIST_MINERALS_ABUNDANT,
	STRING_PLANET_LIST_MINERALS_RICH,
	STRING_PLANET_LIST_MINERALS_ULTRA_RICH
};
const int sizesMap[5] = {
	STRING_PLANET_LIST_SIZE_TINY,
	STRING_PLANET_LIST_SIZE_SMALL,
	STRING_PLANET_LIST_SIZE_MEDIUM,
	STRING_PLANET_LIST_SIZE_LARGE,
	STRING_PLANET_LIST_SIZE_HUGE
};

GalaxyMinimapWidget::GalaxyMinimapWidget(unsigned x, unsigned y,
	unsigned width, unsigned height, GameState *game, int activePlayer,
	const char *archive, unsigned starAssets, unsigned fleetAssets,
	const uint8_t *palette) : Widget(x, y, width, height), _game(game),
	_curFleet(NULL), _selFleet(NULL), _startTick(0), _curStar(-1),
	_selStar(-1), _activePlayer(activePlayer) {

	unsigned i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		_starimg[i] = gameAssets->getImage(archive, starAssets + i,
			palette);
	}

	_freestar = gameAssets->getImage(archive, starAssets + MAX_PLAYERS,
		palette);
	_bhole = gameAssets->getImage(archive, starAssets + MAX_PLAYERS + 2,
		palette);

	for (i = 0; i < MAX_FLEET_OWNERS; i++) {
		_fleetimg[i] = gameAssets->getImage(archive, fleetAssets + i,
			palette);
	}
}

GalaxyMinimapWidget::~GalaxyMinimapWidget(void) {

}

unsigned GalaxyMinimapWidget::starX(unsigned x) {
	return getX() + (x * width()) / _game->_galaxy.width;
}

unsigned GalaxyMinimapWidget::starY(unsigned y) {
	return getY() + (y * height()) / _game->_galaxy.height;
}

unsigned GalaxyMinimapWidget::fleetX(const Fleet *f) {
	unsigned ret = starX(f->getX());
	const Image *img;

	switch (f->getStatus()) {
	case ShipState::InOrbit:
		return ret + 1;

	case ShipState::LeavingOrbit:
		img = getFleetSprite(f);
		return ret - img->width();

	case ShipState::InTransit:
		img = getFleetSprite(f);
		return ret - img->width() / 2;

	default:
		throw std::runtime_error("Invalid fleet state");
	}
}

unsigned GalaxyMinimapWidget::fleetY(const Fleet *f) {
	unsigned ret = starY(f->getY());
	const Image *img;

	switch (f->getStatus()) {
	case ShipState::InOrbit:
	case ShipState::LeavingOrbit:
		return ret - 10;

	case ShipState::InTransit:
		img = getFleetSprite(f);
		return ret - img->height() / 2;

	default:
		throw std::runtime_error("Invalid fleet state");
	}
}

const Image *GalaxyMinimapWidget::getFleetSprite(const Fleet *f) {
	unsigned cls = f->getOwner();

	if (cls < MAX_PLAYERS) {
		cls = _game->_players[cls].color;
	}

	return (const Image*)_fleetimg[cls];
}

const Image *GalaxyMinimapWidget::getStarSprite(const Star *s) {
	unsigned color;

	if (s->spectralClass == BlackHole) {
		return (Image*)_bhole;
	} else if (s->owner < 0) {
		return (Image*)_freestar;
	}

	// FIXME: handle unexplored stars
	color = _game->_players[s->owner].color;
	return (Image*)_starimg[color];
}

void GalaxyMinimapWidget::drawFleet(int x, int y, const Fleet *f,
	unsigned curtick) {

	unsigned frame = 0;
	const Image *img = getFleetSprite(f);

	if (f == _selFleet) {
		frame = (curtick - _startTick) / 60;
		frame %= img->frameCount();
	}

	img->draw(x + fleetX(f), y + fleetY(f), frame);
}

void GalaxyMinimapWidget::drawStar(int x, int y, const Star *s,
	unsigned curtick) {

	unsigned frame = 0;
	const Star *selptr, *curptr;
	const Image *img = getStarSprite(s);
	const uint8_t *color;

	selptr = _selStar < 0 ? NULL : _game->_starSystems + _selStar;
	curptr = _curStar < 0 ? NULL : _game->_starSystems + _curStar;
	x += starX(s->x);
	y += starY(s->y);

	img->draw(x - img->width() / 2, y - img->height() / 2, frame);

	if (s == selptr) {
		frame = (curtick - _startTick) / 200;
		frame %= 2 * STARSEL_FRAMECOUNT - 1;

		if (frame >= STARSEL_FRAMECOUNT) {
			frame = 2 * STARSEL_FRAMECOUNT - frame - 1;
		}

		color = minimapStarSelColors + frame * 3;
		drawRect(x - 5, y - 5, 11, 11, color[0], color[1], color[2]);
	} else if (s == curptr) {
		drawRect(x - 5, y - 5, 11, 11, RGB(0x006000));
	}
}

void GalaxyMinimapWidget::findObject(unsigned x, unsigned y, int *rstar,
	Fleet **rfleet) {

	unsigned i, px, py;
	const BilistNode<Fleet> *fnode;

	*rstar = -1;
	*rfleet = NULL;
	fnode = _game->getMovingFleets();

	for (; fnode; fnode = fnode->next()) {
		if (!fnode->data) {
			continue;
		}

		if (touchesFleet(x, y, fnode->data)) {
			*rfleet = fnode->data;
			return;
		}
	}

	for (i = 0; i < _game->_starSystemCount; i++) {
		const Star *ptr = _game->_starSystems + i;

		fnode = ptr->getOrbitingFleets();

		if (fnode && fnode->data && touchesFleet(x, y,
			fnode->data)) {

			*rfleet = fnode->data;
			return;
		}

		fnode = ptr->getLeavingFleets();

		if (fnode && fnode->data && touchesFleet(x, y,
			fnode->data)) {

			*rfleet = fnode->data;
			return;
		}

		px = starX(ptr->x);
		py = starY(ptr->y);

		if (isInRect(x, y, px - 4, py - 4, 9, 9)) {
			*rstar = i;
			return;
		}
	}
}

int GalaxyMinimapWidget::touchesFleet(unsigned x, unsigned y, const Fleet *f) {
	unsigned fx, fy;
	const Image *img;

	img = getFleetSprite(f);
	fx = fleetX(f);
	fy = fleetY(f);

	return isInRect(x, y, fx, fy, img->width(), img->height());
}

Fleet *GalaxyMinimapWidget::highlightedFleet(void) {
	return _curFleet;
}

Fleet *GalaxyMinimapWidget::selectedFleet(void) {
	return _selFleet;
}

int GalaxyMinimapWidget::highlightedStar(void) {
	return _curStar;
}

int GalaxyMinimapWidget::selectedStar(void) {
	return _selStar;
}

void GalaxyMinimapWidget::highlightFleet(Fleet *f) {
	_curStar = -1;
	_curFleet = f;
}

void GalaxyMinimapWidget::selectFleet(Fleet *f) {
	_selFleet = f;
	_startTick = 0;
}

void GalaxyMinimapWidget::highlightStar(int id) {
	if (id >= _game->_starSystemCount) {
		throw std::invalid_argument("Star ID out of range");
	}

	_curFleet = NULL;
	_curStar = id;
}

void GalaxyMinimapWidget::selectStar(int id) {
	if (id >= _game->_starSystemCount) {
		throw std::invalid_argument("Star ID out of range");
	}

	_selStar = id;
	_startTick = 0;
}

void GalaxyMinimapWidget::setStarHighlightCallback(
	const GuiCallback &callback) {

	_onStarHighlight = callback;
}

void GalaxyMinimapWidget::setStarSelectCallback(const GuiCallback &callback) {
	_onStarSelect = callback;
}

void GalaxyMinimapWidget::setFleetHighlightCallback(
	const GuiCallback &callback) {

	_onFleetHighlight = callback;
}

void GalaxyMinimapWidget::setFleetSelectCallback(const GuiCallback &callback) {
	_onFleetSelect = callback;
}

void GalaxyMinimapWidget::handleMouseMove(int x, int y, unsigned buttons) {
	Fleet *f = NULL;
	int star = -1;

	findObject(x, y, &star, &f);

	if (f && f != _curFleet) {
		highlightFleet(f);
		_onFleetHighlight(x, y);
		return;
	}

	if (star >= 0 && star != _curStar &&
		_game->_starSystems[star].spectralClass != BlackHole) {
		highlightStar(star);
		_onStarHighlight(x, y);
		return;
	}

	Widget::handleMouseMove(x, y, buttons);
}

void GalaxyMinimapWidget::handleMouseUp(int x, int y, unsigned button) {
	Fleet *f = NULL;
	int star = -1;

	if (button != MBUTTON_LEFT) {
		Widget::handleMouseUp(x, y, button);
		return;
	}

	findObject(x, y, &star, &f);

	if (f) {
		selectFleet(f);
		_onFleetSelect(x, y);
		return;
	}

	if (star >= 0) {
		selectStar(star);
		_onStarSelect(x, y);
		return;
	}

	Widget::handleMouseUp(x, y, button);
}

void GalaxyMinimapWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i;
	const BilistNode<Fleet> *fnode;

	if (isHidden()) {
		return;
	}

	if (!_startTick) {
		_startTick = curtick;
	}

	for (i = 0; i < _game->_starSystemCount; i++) {
		const Star *ptr = _game->_starSystems + i;

		drawStar(x, y, ptr, curtick);

		// FIXME: draw draw up to 3 fleets from both groups
		fnode = ptr->getOrbitingFleets();

		if (fnode && fnode->data) {
			drawFleet(x, y, fnode->data, curtick);
		}

		fnode = ptr->getLeavingFleets();

		if (fnode && fnode->data) {
			drawFleet(x, y, fnode->data, curtick);
		}
	}

	for (fnode = _game->getMovingFleets(); fnode; fnode = fnode->next()) {
		if (!fnode->data) {
			continue;
		}

		drawFleet(x, y, fnode->data, curtick);
	}
}

GalaxyView::GalaxyView(GameState *game) : _game(game), _zoom(0), _zoomX(0),
	_zoomY(0), _startTick(0), _activePlayer(-1) {
	uint8_t tpal[PALSIZE];
	const uint8_t *pal;
	const Player *plr;
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

	for (i = 0; i < _game->_playerCount; i++) {
		plr = _game->_players + i;

		// Network players intentionally included
		if (plr->objective != OBJECTIVE_HUMAN || plr->eliminated) {
			continue;
		}

		_turnDoneLights[i] = gameAssets->getImage(GALAXY_ARCHIVE,
			ASSET_GALAXY_TURN_DONE + plr->color, pal);
	}

	_bg = gameAssets->getImage(STARBG_ARCHIVE, ASSET_STARBG, pal);

	for (i = 0, k = 0; i < NEBULA_TYPE_COUNT; i++) {
		for (j = 0; j < GALAXY_ZOOM_LEVELS; j++, k++) {
			_nebulaimg[i][j] = gameAssets->getImage(STARBG_ARCHIVE,
				ASSET_STARBG_NEBULA_IMAGES + k, tpal);
		}
	}

	initWidgets();
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
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_GAME_BUTTON));

	w = createWidget(15, 430, 67, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickColoniesButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_COLONIES_BUTTON, pal, -15, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_COLONIES_BUTTON));

	w = createWidget(90, 430, 67, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickPlanetsButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_PLANETS_BUTTON, pal, -90, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_PLANETS_BUTTON));

	w = createWidget(165, 430, 67, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickFleetsButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_FLEETS_BUTTON, pal, -165, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_FLEETS_BUTTON));

	w = createWidget(310, 430, 70, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickLeadersButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_LEADERS_BUTTON, pal, -4, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_LEADERS_BUTTON));

	w = createWidget(385, 430, 70, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickRacesButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_RACES_BUTTON, pal, -79, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_RACES_BUTTON));

	w = createWidget(460, 430, 70, 44);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickInfoButton));
	w->setClickSprite(MBUTTON_LEFT, new GuiSprite(GALAXY_ARCHIVE,
		ASSET_GALAXY_INFO_BUTTON, pal, -154, -5, 1));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_INFO_BUTTON));

	w = createWidget(244, 428, 55, 16);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickZoomInButton));
	w->setClickSprite(MBUTTON_LEFT, GALAXY_ARCHIVE,
		ASSET_GALAXY_ZOOMIN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_ZOOM_BUTTON));

	w = createWidget(244, 455, 55, 19);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickZoomOutButton));
	w->setClickSprite(MBUTTON_LEFT, GALAXY_ARCHIVE,
		ASSET_GALAXY_ZOOMOUT_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_ZOOM_BUTTON));

	w = createWidget(544, 441, 65, 26);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickTurnButton));
	w->setClickSprite(MBUTTON_LEFT, GALAXY_ARCHIVE,
		ASSET_GALAXY_TURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_TURN_BUTTON));
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
			ptr->networkPlayerId == 0 && !ptr->eliminated) {
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
	fnt = gameFonts->getFont(FONTSIZE_SMALL);
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
	const Player *plr;
	BilistNode<Fleet> *fnode;
	unsigned font_sizes[] = {FONTSIZE_MEDIUM, FONTSIZE_SMALL,
		FONTSIZE_SMALL, FONTSIZE_SMALLER};

	if (!_startTick) {
		_startTick = curtick;
	}

	fnt = gameFonts->getFont(font_sizes[_zoom]);

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
	x = 73;

	for (i = 0; i < _game->_playerCount; i++) {
		plr = _game->_players + i;

		if (plr->objective != OBJECTIVE_HUMAN || plr->eliminated ||
			!plr->playerDoneFlags) {
			continue;
		}

		img = (Image*)_turnDoneLights[i];
		frame = (curtick - _startTick) / GALAXY_ANIM_SPEED;
		frame %= img->frameCount();
		img->draw(x, 5, frame);
		x += img->width();
	}

	redrawSidebar(curtick);
	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void GalaxyView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _bg->palette());
}

void GalaxyView::clickGameMenu(int x, int y, int arg) {
	new MainMenuWindow(this, _game);
}

void GalaxyView::clickColoniesButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickPlanetsButton(int x, int y, int arg) {
	gui_stack->push(new PlanetsListView(_game, _activePlayer));
}

void GalaxyView::clickFleetsButton(int x, int y, int arg) {
	gui_stack->push(new FleetListView(_game, _activePlayer));
}

void GalaxyView::clickLeadersButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickRacesButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickInfoButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickZoomInButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickZoomOutButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
}

void GalaxyView::clickTurnButton(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented yet");
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
			ptr->networkPlayerId != 0 || ptr->eliminated) {
			continue;
		}

		flag_id = ptr->picture * PLAYER_COUNT + ptr->color;
		_playerFlags[_playerCount] = gameAssets->getImage(
			MULTIPLAYER_ARCHIVE, ASSET_PSELECT_FLAGS + flag_id,
			pal);
		_humans[_playerCount++] = i;
	}

	// Row count is intentionally off by one
	_y = SCREEN_HEIGHT - _header->height() - _footer->height();
	_y = (_y - _playerCount * _row->height()) / 2;

	initWidgets();
}

void SelectPlayerView::initWidgets(void) {
	Widget *w;
	unsigned i, x, h;

	h = _row->height();
	x = (SCREEN_WIDTH - _header->width()) / 2;

	for (i = 0; i < _playerCount; i++) {
		if (_game->_players[_humans[i]].playerDoneFlags) {
			continue;
		}

		// Player flag
		w = createWidget(x + 37, _y + 63 + i * h, 34, 36);
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
		w = createWidget(x + 104, _y + 66 + i * h, 335, 30);
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

	fnt = gameFonts->getFont(FONTSIZE_BIG);
	h = _row->height();
	x = (SCREEN_WIDTH - _header->width()) / 2;
	y = _y;

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

PlanetsListView::PlanetsListView(const GameState *game, int activePlayer) : _game(game), _activePlayer(activePlayer) {
	_planetAssets = gameAssets->getImage(PLANET_ARCHIVE, ASSET_PLANETLIST_BG);

	// Load all planet icons: each climate holds 5 different sizes, so Toxic climate starts at 0, Radiated at 5, etc
	for (int assetId = 0; assetId < 50; assetId++) { 
		// The planet icons go from offset 26 to offset 26 + 50 = 75
		_planetIcons[assetId] = gameAssets->getImage(PLANET_ARCHIVE, assetId + 26, _planetAssets->palette());
	}

	initWidgets();
}

void PlanetsListView::initWidgets(void) {
	Widget *w = NULL;

	// Create the return button
	w = createWidget(453, 440, 153, 18);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::clickReturn));
	w->setClickSprite(MBUTTON_LEFT, PLANET_ARCHIVE, ASSET_PLANETLIST_RETURN_BUTTON,
		_planetAssets->palette(), 1);
}

void PlanetsListView::redraw(unsigned curtick) {
	Font *fnt, *smallFnt;
	const uint8_t *pal;
	int x, y, displayedPlanet = 1;
	char buf[32];
	const int climateOffset = 5,
			  mineralsOffset = 5,
			  sizeOffset = 5,
			  nameOffset = 10,
			  rowHeight = 55,
			  planetListMaxItems = 8,
			  offset = 10;
	unsigned color = FONT_COLOR_PLANET_LIST;

	clearScreen();
	// Render the planet list background
	_planetAssets->draw(0, 0);

	fnt = gameFonts->getFont(FONTSIZE_SMALL);
	smallFnt = gameFonts->getFont(FONTSIZE_SMALLER);

	for (uint16_t i=0; i < _game->_planetCount; i++) {
		const Planet *planet = &_game->_planets[i];
		if (planet->colony >= 0 && _game->_colonies[planet->colony].owner == _activePlayer) {
			continue;
		}

		// Planet climate
		fnt->centerText(138, rowHeight * displayedPlanet - climateOffset, color, gameLang->estrings(planetClimateMap[planet->climate]));
		smallFnt->centerText(138, rowHeight * displayedPlanet + offset, color, "0 Food"); // FIXME: Pull real data

		// Planet Gravity
		fnt->centerText(220, rowHeight * displayedPlanet, color, gameLang->estrings(planetGravityMap[planet->gravity]));

		// Minerals
		fnt->centerText(306, rowHeight * displayedPlanet - mineralsOffset, color, gameLang->estrings(mineralsMap[planet->minerals]));
		smallFnt->centerText(306, rowHeight * displayedPlanet + offset, color, "3 prod/worker"); // FIXME: Pull real data

		// Planet size
		x = 50;
		fnt->centerText(385, rowHeight * displayedPlanet - sizeOffset, color, gameLang->estrings(sizesMap[planet->size]));
		smallFnt->centerText(385, rowHeight * displayedPlanet + offset, color, "4 max pop"); // FIXME: Pull real data

		// Render the planet icon
		_planetIcons[planet->climate * 5 + planet->size]->draw(x - planet->size, (rowHeight * displayedPlanet) - 10);

		// Planet name
		sprintf(buf, "%s %s", _game->_starSystems[planet->star].name, romanNumbers[planet->orbit]);
		fnt->centerText(60, rowHeight * displayedPlanet + nameOffset, color, buf);

		displayedPlanet++;
	}

	redrawWidgets(0, 0, curtick);
}

void PlanetsListView::clickReturn(int x, int y, int arg) {
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

	initWidgets(pal);
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
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_SAVE));

	// Load Game button
	w = createWidget(147, 43, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickLoad));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_LOAD,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_LOAD));

	// New Game button
	w = createWidget(40, 88, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickNew));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_NEW,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_NEWGAME));

	// Quit Game button
	w = createWidget(147, 88, 91, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickQuit));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_QUIT,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_QUIT));

	// Settings button
	w = createWidget(40, 307, 91, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickSettings));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
		ASSET_GAMEMENU_SETTINGS, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_SETTINGS));

	// Return button
	w = createWidget(151, 307, 91, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod<GuiWindow>(*this, &MainMenuWindow::close));
	w->setClickSprite(MBUTTON_LEFT, GAMEMENU_ARCHIVE,
		ASSET_GAMEMENU_RETURN, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_RETURN));
}

void MainMenuWindow::redraw(unsigned curtick) {
	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);
}

void MainMenuWindow::showHelp(int x, int y, int arg) {
	ImageAsset img = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);

	new MessageBoxWindow(_parent, arg, img->palette());
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
