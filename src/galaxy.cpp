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
#include <cmath>
#include "colony.h"
#include "gamestate.h"
#include "screen.h"
#include "guimisc.h"
#include "mainmenu.h"
#include "lang.h"
#include "ships.h"
#include "officer.h"
#include "tech.h"
#include "galaxy.h"

#define STARSEL_FRAMECOUNT 6

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

#define ASSET_GALAXY_STARWIDGET_COLONY_ICONS 35
#define ASSET_GALAXY_STARWIDGET_RETICLE 75
#define ASSET_GALAXY_STARWIDGET_STAR_IMAGES 83
#define ASSET_GALAXY_STARWIDGET_ORBIT_RINGS 90
#define ASSET_GALAXY_STARWIDGET_ASTEROIDS 91
#define ASSET_GALAXY_STARWIDGET_PLANET_IMAGES 92
#define ASSET_GALAXY_STARWIDGET_GAS_GIANT_IMAGE 142
#define ASSET_GALAXY_STARWIDGET_SHADOW 143
#define SHADOW_COLOR_COUNT 10

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
#define STAR_ANIM_SPEED 80
#define PLANET_ANIM_SPEED 60

#define PSELECT_BUFSIZE 128
#define PSELECT_ANIM_LENGTH 8

#define PLANET_ARCHIVE "plntsum.lbx"
#define ASSET_PLANETLIST_BG 0
#define ASSET_PLANETLIST_CLIMATE_SORT 1
#define ASSET_PLANETLIST_MINERALS_SORT 2
#define ASSET_PLANETLIST_SIZE_SORT 3
#define ASSET_PLANETLIST_ENEMY_TOGGLE 4
#define ASSET_PLANETLIST_GRAVITY_TOGGLE 5
#define ASSET_PLANETLIST_ENV_TOGGLE 6
#define ASSET_PLANETLIST_MINERAL_TOGGLE 7
#define ASSET_PLANETLIST_RANGE_TOGGLE 8
#define ASSET_PLANETLIST_COLONY_TOGGLE 9
#define ASSET_PLANETLIST_OUTPOST_TOGGLE 10
#define ASSET_PLANETLIST_SCROLL_UP_BUTTON 11
#define ASSET_PLANETLIST_SCROLL_DOWN_BUTTON 12
#define ASSET_PLANETLIST_RETURN_BUTTON 14
#define ASSET_PLANETLIST_STAR_IMAGES 15
#define ASSET_PLANETLIST_PLANET_IMAGES 26
#define ASSET_PLANETLIST_SHIP_IMAGES 76

#define PLANET_LIST_SCROLL_WIDTH 9
#define PLANET_LIST_ROWS 8
#define PLANET_LIST_FIRST_ROW 36
#define PLANET_LIST_ROW_HEIGHT 50
#define PLANET_LIST_ROW_DIST 55
#define PLANET_LIST_SHIP_YOFF 36
#define PLANET_LIST_COLONY_X 88
#define PLANET_LIST_OUTPOST_X 18

#define COLONY_ARCHIVE "colsum.lbx"
#define ASSET_COLONYLIST_BG 0
#define ASSET_COLONYLIST_RETURN_BUTTON 1

#define COLONY_LIST_ROWS 10
#define COLONY_LIST_FIRST_ROW 39
#define COLONY_LIST_ROW_HEIGHT 26
#define COLONY_LIST_ROW_DIST 5
#define COLONY_LIST_ROW_LEFT_PADDING 55
#define COLONY_LIST_ROW_BOTTOM_PADDING 5
#define COLONY_LIST_DETAILS_LEFT_PADDING 15

static const uint8_t starmapHighlightColors[(MAX_PLAYERS + 1) * 3] {
	RGB(0xfc0000), RGB(0xd4c418), RGB(0x209c1c), RGB(0xc8c8c8),
	RGB(0x305ca0), RGB(0xa47050), RGB(0x8c6098), RGB(0xd0680c),
	RGB(0x8c9090)
};

static const uint8_t minimapStarSelColors[STARSEL_FRAMECOUNT * 3] = {
	RGB(0x043804), RGB(0x004c00), RGB(0x006000), RGB(0x087008),
	RGB(0x2c8024), RGB(0x489038)
};

static const uint8_t planetListScrollTexture[PLANET_LIST_SCROLL_WIDTH * 3] = {
	RGB(0x9c9c9c), RGB(0x949898), RGB(0x949898), RGB(0x8c9090),
	RGB(0x8c9090), RGB(0x888888), RGB(0x888888), RGB(0x808080),
	RGB(0x808080),
};

static const uint8_t planetInfoFrame[] = {
	RGB(0x5c7ca0), RGB(0x506c90), RGB(0x384c6c), RGB(0x445c80)
};

static const unsigned galaxy_fontanim[GALAXY_ANIM_LENGTH] = {
	1, 2, 3, 4, 3, 2, 1, 0
};

static const int orbitRadiusX[MAX_ORBITS] = {46, 71, 95, 120, 142};

static const int orbitRadiusY[MAX_ORBITS] = {25, 38, 51, 64, 76};

const int planetClimateMap[PLANET_CLIMATE_COUNT] = {
	ESTR_CLIMATE_TOXIC,
	ESTR_CLIMATE_RADIATED,
	ESTR_CLIMATE_BARREN,
	ESTR_CLIMATE_DESERT,
	ESTR_CLIMATE_TUNDRA,
	ESTR_CLIMATE_OCEAN,
	ESTR_CLIMATE_SWAMP,
	ESTR_CLIMATE_ARID,
	ESTR_CLIMATE_TERRAN,
	ESTR_CLIMATE_GAIA
};

const int mineralsMap[PLANET_MINERALS_COUNT] = {
	ESTR_LMINERALS_ULTRA_POOR,
	ESTR_LMINERALS_POOR,
	ESTR_LMINERALS_ABUNDANT,
	ESTR_LMINERALS_RICH,
	ESTR_LMINERALS_ULTRA_RICH
};

const int sizesMap[PLANET_SIZE_COUNT] = {
	ESTR_PLANET_SIZE_TINY,
	ESTR_PLANET_SIZE_SMALL,
	ESTR_PLANET_SIZE_MEDIUM,
	ESTR_PLANET_SIZE_LARGE,
	ESTR_PLANET_SIZE_HUGE
};

const unsigned planetSpriteSizes[PLANET_SIZE_COUNT + 1] = {
	19, 20, 22, 22, 25, 26
};

const unsigned planetShadowOffsets[PLANET_CLIMATE_COUNT + 1] = {
	0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0
};

static const unsigned planetListFontColors[MAX_PLAYERS + 1] = {
	FONT_COLOR_PLAYER_RED4,
	FONT_COLOR_PLAYER_YELLOW4,
	FONT_COLOR_PLAYER_GREEN4,
	FONT_COLOR_PLANET_LIST_SILVER,
	FONT_COLOR_PLANET_LIST_BLUE,
	FONT_COLOR_PLANET_LIST_BROWN,
	FONT_COLOR_PLAYER_PURPLE4,
	FONT_COLOR_PLAYER_ORANGE4,
	FONT_COLOR_PLANET_LIST
};

static const unsigned planetListFontColorsBright[MAX_PLAYERS + 1] = {
	FONT_COLOR_PLAYER_RED5,
	FONT_COLOR_PLAYER_YELLOW5,
	FONT_COLOR_PLAYER_GREEN5,
	FONT_COLOR_PLANET_LIST_SILVER_BRIGHT,
	FONT_COLOR_PLANET_LIST_BLUE_BRIGHT,
	FONT_COLOR_PLANET_LIST_BROWN_BRIGHT,
	FONT_COLOR_PLAYER_PURPLE5,
	FONT_COLOR_PLAYER_ORANGE5,
	FONT_COLOR_PLANET_LIST_BRIGHT
};

static const unsigned galaxyStarColors[MAX_PLAYERS] = {
	FONT_COLOR_STAR_RED1, FONT_COLOR_STAR_YELLOW1, FONT_COLOR_STAR_GREEN1,
	FONT_COLOR_STAR_SILVER1, FONT_COLOR_STAR_BLUE1, FONT_COLOR_STAR_BROWN1,
	FONT_COLOR_STAR_PURPLE1, FONT_COLOR_STAR_ORANGE1
};

static const unsigned defensiveBuildings[] = {
	BUILDING_STAR_FORTRESS,
	BUILDING_BATTLESTATION,
	BUILDING_STAR_BASE,
	BUILDING_MISSILE_BASE,
	BUILDING_GROUND_BATTERIES,
	BUILDING_STELLAR_CONVERTER,
	BUILDING_FIGHTER_GARRISON,
	0
};

StarmapWidget::StarmapWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, GameState *game, unsigned activePlayer,
	const char *archive, unsigned starAssets, const uint8_t *palette) :
	Widget(x, y, width, height), _curStar(-1), _selStar(-1), _startTick(0),
	_game(game), _activePlayer(activePlayer),
	_starSelFilter(SELFILTER_ANY), _starHlFilter(SELFILTER_ANY) {

	unsigned i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		_starimg[i] = gameAssets->getImage(archive, starAssets + i,
			palette);
	}

	_freestar = gameAssets->getImage(archive, starAssets + MAX_PLAYERS,
		palette);
	_bhole = gameAssets->getImage(archive, starAssets + MAX_PLAYERS + 2,
		palette);
}

StarmapWidget::~StarmapWidget(void) {

}

unsigned StarmapWidget::starX(unsigned x) const {
	return getX() + (x * width()) / _game->_galaxy.width;
}

unsigned StarmapWidget::starY(unsigned y) const {
	return getY() + (y * height()) / _game->_galaxy.height;
}

const Image *StarmapWidget::getStarSprite(const Star *s) {
	unsigned color;

	if (s->spectralClass == BlackHole) {
		return (Image*)_bhole;
	} else if (!_game->isStarExplored(s, _activePlayer) || s->owner < 0) {
		return (Image*)_freestar;
	}

	color = _game->_players[s->owner].color;
	return (Image*)_starimg[color];
}

void StarmapWidget::drawStar(int x, int y, const Star *s, unsigned curtick) {
	unsigned frame = 0;
	const Image *img = getStarSprite(s);

	x += starX(s->x);
	y += starY(s->y);

	if (curtick) {
		frame = loopFrame(curtick-_startTick, 60, img->frameCount());
	}

	img->draw(x - img->width() / 2, y - img->height() / 2, frame);
}

int StarmapWidget::findStar(int x, int y) const {
	unsigned i, px, py;

	for (i = 0; i < _game->_starSystemCount; i++) {
		const Star *ptr = _game->_starSystems + i;

		px = starX(ptr->x);
		py = starY(ptr->y);

		if (isInRect(x, y, px - 4, py - 4, 9, 9)) {
			return i;
		}
	}

	return -1;
}

int StarmapWidget::isStarAllowed(int id, SelectionFilter filter) {
	if (id < 0) {
		return 1;
	} else if (id >= _game->_starSystemCount) {
		throw std::invalid_argument("Star ID out of range");
	}

	switch (filter) {
	case SELFILTER_ANY:
		return 1;

	case SELFILTER_OWNED:
		return _game->_starSystems[id].hasColony & (1 << _activePlayer);

	default:
		return 0;
	}
}

void StarmapWidget::onStarHighlight(int x, int y) {
	_onStarHighlight(x, y);
}

void StarmapWidget::onStarSelect(int x, int y) {
	_onStarSelect(x, y);
}

int StarmapWidget::highlightedStar(void) {
	return _curStar;
}

int StarmapWidget::selectedStar(void) {
	return _selStar;
}

void StarmapWidget::highlightStar(int id) {
	if (id >= _game->_starSystemCount) {
		throw std::invalid_argument("Star ID out of range");
	}

	if (!isStarAllowed(id, _starHlFilter)) {
		return;
	}

	if (_curStar != id) {
		_curStar = id;
		_startTick = 0;
	}
}

void StarmapWidget::selectStar(int id) {
	if (id >= _game->_starSystemCount) {
		throw std::invalid_argument("Star ID out of range");
	}

	if (!isStarAllowed(id, _starSelFilter)) {
		return;
	}

	_selStar = id;
}

void StarmapWidget::setStarHighlightCallback(const GuiCallback &callback) {
	_onStarHighlight = callback;
}

void StarmapWidget::setStarSelectCallback(const GuiCallback &callback) {
	_onStarSelect = callback;
}

void StarmapWidget::setStarFilter(SelectionFilter select,
	SelectionFilter highlight) {

	_starSelFilter = select;
	_starHlFilter = highlight;

	if (!isStarAllowed(_selStar, _starSelFilter)) {
		_selStar = -1;
	}

	if (!isStarAllowed(_curStar, _starHlFilter)) {
		_curStar = -1;
	}
}

void StarmapWidget::handleMouseMove(int x, int y, unsigned buttons) {
	int star = findStar(x, y);

	if (star >= 0 && star != _curStar &&
		isStarAllowed(star, _starHlFilter)) {

		highlightStar(star);
		_onStarHighlight(x, y);
		return;
	}

	Widget::handleMouseMove(x, y, buttons);
}

void StarmapWidget::handleMouseUp(int x, int y, unsigned button) {
	int star = -1;

	if (button != MBUTTON_LEFT) {
		Widget::handleMouseUp(x, y, button);
		return;
	}

	star = findStar(x, y);

	if (star >= 0 && isStarAllowed(star, _starSelFilter)) {
		selectStar(star);
		_onStarSelect(x, y);
		return;
	}

	Widget::handleMouseUp(x, y, button);
}

void StarmapWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, sx, sy, color = MAX_PLAYERS;
	const uint8_t *cdata;

	if (isHidden()) {
		return;
	}

	if (!_startTick) {
		_startTick = curtick;
	}

	for (i = 0; i < _game->_starSystemCount; i++) {
		const Star *ptr = _game->_starSystems + i;

		if (_curStar == (int)i) {
			sx = starX(ptr->x);
			sy = starY(ptr->y);

			if (_game->isStarExplored(ptr, _activePlayer) &&
				ptr->owner >= 0) {

				color = _game->_players[ptr->owner].color;
			}

			cdata = starmapHighlightColors + 3 * color;
			drawStar(x, y, ptr, curtick);
			gameScreen->drawRect(x + sx - 4, y + sy - 4, 9, 9,
				cdata[0], cdata[1], cdata[2]);
		} else {
			drawStar(x, y, ptr, 0);
		}
	}
}

GalaxyMinimapWidget::GalaxyMinimapWidget(unsigned x, unsigned y,
	unsigned width, unsigned height, GameState *game,
	unsigned activePlayer, const char *archive, unsigned starAssets,
	unsigned fleetAssets, const uint8_t *palette) :
	StarmapWidget(x, y, width, height, game, activePlayer, archive,
	starAssets, palette), _fleetSelFilter(SELFILTER_ANY),
	_fleetHlFilter(SELFILTER_ANY), _curFleet(NULL), _selFleet(NULL),
	_startTick(0) {

	unsigned i;

	for (i = 0; i < MAX_FLEET_OWNERS; i++) {
		_fleetimg[i] = gameAssets->getImage(archive, fleetAssets + i,
			palette);
	}
}

GalaxyMinimapWidget::~GalaxyMinimapWidget(void) {

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
	unsigned color = f->getColor();

	return (const Image*)_fleetimg[color];
}

void GalaxyMinimapWidget::drawFleet(int x, int y, const Fleet *f,
	unsigned curtick) {

	unsigned frame = 0;
	const Image *img = getFleetSprite(f);

	if (f == _selFleet) {
		frame = loopFrame(curtick - _startTick, 60, img->frameCount());
	}

	img->draw(x + fleetX(f), y + fleetY(f), frame);
}

void GalaxyMinimapWidget::drawStar(int x, int y, const Star *s,
	unsigned curtick) {

	unsigned frame = 0;
	int curstar, selstar;
	const Star *selptr, *curptr;
	const uint8_t *color;

	StarmapWidget::drawStar(x, y, s, 0);

	selstar = selectedStar();
	selptr = selstar < 0 ? NULL : _game->_starSystems + selstar;
	curstar = highlightedStar();
	curptr = curstar < 0 ? NULL : _game->_starSystems + curstar;
	x += starX(s->x);
	y += starY(s->y);

	if (s == selptr) {
		frame = bounceFrame(curtick - _startTick, 200,
			STARSEL_FRAMECOUNT);
		color = minimapStarSelColors + frame * 3;
		gameScreen->drawRect(x - 5, y - 5, 11, 11, color[0], color[1],
			color[2]);
	} else if (s == curptr) {
		gameScreen->drawRect(x - 5, y - 5, 11, 11, RGB(0x006000));
	}
}

void GalaxyMinimapWidget::findObject(unsigned x, unsigned y, int *rstar,
	Fleet **rfleet, SelectionFilter fleetFilter) {

	unsigned i, px, py;
	const BilistNode<Fleet> *fnode;

	*rstar = -1;
	*rfleet = NULL;
	fnode = _game->getMovingFleets();

	for (; fnode; fnode = fnode->next()) {
		if (!fnode->data) {
			continue;
		}

		if (touchesFleet(x, y, fnode->data, fleetFilter)) {
			*rfleet = fnode->data;
			return;
		}
	}

	for (i = 0; i < _game->_starSystemCount; i++) {
		const Star *ptr = _game->_starSystems + i;

		fnode = ptr->getOrbitingFleets();

		if (fnode && fnode->data && touchesFleet(x, y,
			fnode->data, fleetFilter)) {

			*rfleet = fnode->data;
			return;
		}

		fnode = ptr->getLeavingFleets();

		if (fnode && fnode->data && touchesFleet(x, y,
			fnode->data, fleetFilter)) {

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

int GalaxyMinimapWidget::touchesFleet(unsigned x, unsigned y, const Fleet *f,
	SelectionFilter filter) {
	unsigned fx, fy;
	const Image *img;

	if (!isFleetAllowed(f, filter)) {
		return 0;
	}

	img = getFleetSprite(f);
	fx = fleetX(f);
	fy = fleetY(f);

	return isInRect(x, y, fx, fy, img->width(), img->height());
}

int GalaxyMinimapWidget::isFleetAllowed(const Fleet *f,
	SelectionFilter filter) {

	if (!f) {
		return 1;
	}

	switch (filter) {
	case SELFILTER_ANY:
		return 1;

	case SELFILTER_OWNED:
		return f->getOwner() == _activePlayer;

	default:
		return 0;
	}
}

Fleet *GalaxyMinimapWidget::highlightedFleet(void) {
	return _curFleet;
}

Fleet *GalaxyMinimapWidget::selectedFleet(void) {
	return _selFleet;
}

void GalaxyMinimapWidget::highlightFleet(Fleet *f) {
	if (!isFleetAllowed(f, _fleetHlFilter)) {
		return;
	}

	StarmapWidget::highlightStar(-1);
	_curFleet = f;
}

void GalaxyMinimapWidget::selectFleet(Fleet *f) {
	if (!isFleetAllowed(f, _fleetSelFilter)) {
		return;
	}

	if (f != _selFleet) {
		_selFleet = f;
		_startTick = 0;
	}
}

void GalaxyMinimapWidget::highlightStar(int id) {
	if (isStarAllowed(id, _starHlFilter)) {
		StarmapWidget::highlightStar(id);
		_curFleet = NULL;
	}
}

void GalaxyMinimapWidget::selectStar(int id) {
	if (isStarAllowed(id, _starSelFilter) && id != selectedStar()) {
		StarmapWidget::selectStar(id);
		_startTick = 0;
	}
}

void GalaxyMinimapWidget::setFleetHighlightCallback(
	const GuiCallback &callback) {

	_onFleetHighlight = callback;
}

void GalaxyMinimapWidget::setFleetSelectCallback(const GuiCallback &callback) {
	_onFleetSelect = callback;
}

void GalaxyMinimapWidget::setFleetFilter(SelectionFilter select,
	SelectionFilter highlight) {

	_fleetSelFilter = select;
	_fleetHlFilter = highlight;

	if (!isFleetAllowed(_selFleet, _fleetSelFilter)) {
		_selFleet = NULL;
	}

	if (!isFleetAllowed(_curFleet, _fleetHlFilter)) {
		_curFleet = NULL;
	}
}

void GalaxyMinimapWidget::handleMouseMove(int x, int y, unsigned buttons) {
	Fleet *f = NULL;
	int star = -1;

	findObject(x, y, &star, &f, _fleetHlFilter);

	if (f && f != _curFleet) {
		highlightFleet(f);
		_onFleetHighlight(x, y);
		return;
	}

	if (star >= 0 && star != highlightedStar() &&
		isStarAllowed(star, _starHlFilter) &&
		_game->_starSystems[star].spectralClass != BlackHole) {

		highlightStar(star);
		onStarHighlight(x, y);
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

	findObject(x, y, &star, &f, _fleetSelFilter);

	if (f) {
		selectFleet(f);
		_onFleetSelect(x, y);
		return;
	}

	if (star >= 0 && isStarAllowed(star, _starSelFilter)) {
		selectStar(star);
		onStarSelect(x, y);
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

StarSystemWidget::StarSystemWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, GameState *game, unsigned activePlayer, unsigned star,
	const Image *bg) : Widget(x, y, width, height), _reticle(NULL),
	_info(NULL), _bg(bg), _game(game), _activePlayer(activePlayer),
	_startTick(0), _curOrbit(-1) {

	ImageAsset palImg, reticleImg;
	unsigned i, j, k;
	const uint8_t *pal, *rpal[MAX_PLAYERS + 1];
	uint8_t *palbuf;
	unsigned rpal_offsets[MAX_PLAYERS + 1] = {
		0, 25, 37, 215, 245, 75, 228, 12, 189
	};

	palImg = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = palImg->palette();
	palbuf = new uint8_t[2 * PALSIZE];
	memcpy(palbuf, pal, PALSIZE);
	memcpy(palbuf + PALSIZE, pal, PALSIZE);

	for (i = 0; i < MAX_PLAYERS + 1; i++) {
		rpal[i] = palbuf + 4 * rpal_offsets[i];
	}

	try {
		reticleImg = gameAssets->getImage(GALAXY_ARCHIVE,
			ASSET_GALAXY_STARWIDGET_RETICLE, rpal, MAX_PLAYERS + 1);
	} catch (...) {
		delete[] palbuf;
		throw;
	}

	memset(palbuf, 0, PALSIZE);

	// FIXME: analyze original game and calculate better shadow palette
	for (i = 1; i < SHADOW_COLOR_COUNT; i++) {
		palbuf[4 * i] = 17 * i;
	}

	try {
		for (i = 0; i < PLANET_SIZE_COUNT; i++) {
			_planetShadow[i] = gameAssets->getImage(GALAXY_ARCHIVE,
				ASSET_GALAXY_STARWIDGET_SHADOW + i, palbuf);
		}
	} catch (...) {
		delete[] palbuf;
		throw;
	}

	delete[] palbuf;

	for (i = 0; i < STAR_TYPE_COUNT; i++) {
		_starImg[i] = gameAssets->getImage(GALAXY_ARCHIVE,
			ASSET_GALAXY_STARWIDGET_STAR_IMAGES + i, pal);
	}

	for (i = 0, k = 0; i < PLANET_CLIMATE_COUNT; i++) {
		for (j = 0; j < PLANET_SIZE_COUNT; j++, k++) {
			_planetImg[i][j] = gameAssets->getImage(GALAXY_ARCHIVE,
				ASSET_GALAXY_STARWIDGET_PLANET_IMAGES + k, pal);
		}
	}

	// FIXME: there are two sets of icons, one filled and one not, why?
	for (i = 0; i < MAX_PLAYERS; i++) {
		_colonyIcons[i] = gameAssets->getImage(GALAXY_ARCHIVE,
			ASSET_GALAXY_STARWIDGET_COLONY_ICONS + i, pal);
	}

	_gasGiant = gameAssets->getImage(GALAXY_ARCHIVE,
		ASSET_GALAXY_STARWIDGET_GAS_GIANT_IMAGE, pal);
	_orbits = gameAssets->getImage(GALAXY_ARCHIVE,
		ASSET_GALAXY_STARWIDGET_ORBIT_RINGS, pal);
	_asteroids = gameAssets->getImage(GALAXY_ARCHIVE,
		ASSET_GALAXY_STARWIDGET_ASTEROIDS, pal);

	setStar(star);

	_reticle = new GuiSprite((Image*)reticleImg, -(reticleImg->width() / 2),
		-(reticleImg->height() / 2), ANIM_LOOP, 0, 0, 0, 0,
		PLANET_ANIM_SPEED);

	if (bg) {
		try {
			gameAssets->takeAsset(bg);
		} catch (...) {
			delete _reticle;
			throw;
		}
	}
}

StarSystemWidget::~StarSystemWidget(void) {
	delete _info;
	delete _reticle;

	if (_bg) {
		gameAssets->freeAsset(_bg);
	}
}

double StarSystemWidget::orbitAngle(unsigned orbit) const {
	// FIXME: Analyze and implement
	return 0;
}

int StarSystemWidget::orbitX(unsigned orbit) const {
	return cos(orbitAngle(orbit)) * orbitRadiusX[orbit];
}

int StarSystemWidget::orbitY(unsigned orbit) const {
	return -sin(orbitAngle(orbit)) * orbitRadiusY[orbit];
}

int StarSystemWidget::findPlanet(int x, int y) const {
	unsigned i, ps, pw, ph;
	int planet_id, orbit, px, py, cx = width() / 2, cy = height() / 2;
	const Planet *pptr;

	for (i = 0; i < MAX_ORBITS; i++) {
		orbit = _drawlist[MAX_ORBITS - i - 1];

		if (orbit < 0) {
			continue;
		}

		planet_id = _game->getOrbitingPlanetID(_star, orbit);
		pptr = _game->_planets + planet_id;

		if (pptr->type == PlanetType::ASTEROIDS) {
			if (orbit) {
				pw = orbitRadiusX[orbit] +
					orbitRadiusX[orbit - 1];
				ph = orbitRadiusY[orbit] +
					orbitRadiusY[orbit - 1];
			} else {
				pw = 3 * orbitRadiusX[orbit] -
					orbitRadiusX[orbit + 1];
				ph = 3 * orbitRadiusY[orbit] -
					orbitRadiusY[orbit + 1];
			}

			if (isInEllipse(x, y, cx, cy, pw / 2, ph / 2)) {
				continue;
			}

			if (orbit < MAX_ORBITS - 1) {
				pw = orbitRadiusX[orbit] +
					orbitRadiusX[orbit + 1];
				ph = orbitRadiusY[orbit] +
					orbitRadiusY[orbit + 1];
			} else {
				pw = 3 * orbitRadiusX[orbit] -
					orbitRadiusX[orbit - 1];
				ph = 3 * orbitRadiusY[orbit] -
					orbitRadiusY[orbit - 1];
			}

			if (!isInEllipse(x, y, cx, cy, pw / 2, ph / 2)) {
				continue;
			}

			return orbit;
		}

		if (pptr->type == PlanetType::HABITABLE) {
			ps = planetSpriteSizes[pptr->size];
		} else {
			ps = planetSpriteSizes[PLANET_SIZE_COUNT];
		}

		ps /= 2;
		px = cx + orbitX(orbit);
		py = cy + orbitY(orbit);

		if (isInEllipse(x, y, px, py, ps, ps)) {
			return orbit;
		}
	}

	return -1;
}

void StarSystemWidget::updateDrawlist(void) {
	unsigned i, j, k, pcount;
	int tmp, newlist[MAX_ORBITS], ypos[MAX_ORBITS] = {0};
	const Planet *planets[MAX_ORBITS] = {NULL};
	const Star *sptr;

	sptr = _game->_starSystems + _star;

	for (i = 0, pcount = 0; i < MAX_ORBITS; i++) {
		newlist[i] = -1;

		if (sptr->planetIndex[i] < 0) {
			continue;
		}

		planets[i] = _game->_planets + sptr->planetIndex[i];

		if (planets[i]->type != PlanetType::ASTEROIDS) {
			ypos[i] = orbitY(i);
			newlist[pcount++] = i;
		}
	}

	// bubble sort the planets (without asteroids)
	for (i = 0; i < pcount - 1; i++) {
		for (j = 1; j < pcount - i; j++) {
			if (ypos[newlist[j - 1]] > ypos[newlist[j]]) {
				tmp = newlist[j];
				newlist[j] = newlist[j - 1];
				newlist[j - 1] = tmp;
			}
		}
	}

	// insert asteroids
	for (i = 0; i < MAX_ORBITS; i++) {
		tmp = j = MAX_ORBITS - i - 1;

		if (!planets[j] || planets[j]->type != PlanetType::ASTEROIDS) {
			continue;
		}

		for (j = 0; j < pcount; j++) {
			if ((newlist[j] == tmp - 1 && ypos[newlist[j]] < 0) ||
				(newlist[j] == tmp + 1 &&
				ypos[newlist[j]] > 0)) {
				break;
			}
		}

		for (k = pcount++; k > j; k--) {
			newlist[k] = newlist[k - 1];
		}

		newlist[j] = tmp;
	}

	for (i = 0; i < MAX_ORBITS; i++) {
		_drawlist[i] = newlist[i];
	}
}

void StarSystemWidget::setStar(unsigned star) {
	unsigned idx, exp;
	TextLayout *newinfo = NULL, *oldinfo = _info;

	if (star >= _game->_starSystemCount) {
		throw std::out_of_range("Invalid star ID");
	} else if (_game->_starSystems[star].spectralClass >= BlackHole) {
		throw std::out_of_range("Invalid star type");
	}

	exp = _game->isStarExplored(star, _activePlayer) >= STAR_CHARTED;

	if (!exp) {
		idx = HSTR_SPECTRAL_CLASS_DESCRIPTIONS;
		idx += _game->_starSystems[star].spectralClass;
		newinfo = new TextLayout;
		newinfo->setFont(FONTSIZE_MEDIUM, FONT_COLOR_STAR_DESCRIPTION,
			1, OUTLINE_NONE, 2);
		newinfo->appendText(gameLang->hstrings(idx), 0, 0,
			width() - 34, ALIGN_JUSTIFY);
	}

	for (idx = 0; idx < MAX_ORBITS; idx++) {
		_drawlist[idx] = -1;
	}

	_star = star;
	_info = newinfo;
	_explored = exp;
	_startTick = 0;
	highlightOrbit(-1);

	if (oldinfo) {
		oldinfo->discard();
	}

	updateDrawlist();
}

void StarSystemWidget::highlightOrbit(int orbit) {
	unsigned i, y, climate, lineheight, maxwidth = width();
	int planet_id;
	const Star *sptr = _game->_starSystems + _star;
	const Planet *pptr;
	const Colony *cptr = NULL;
	const char *str;
	TextLayout *newinfo = NULL, *oldinfo = _info;
	StringBuffer buf, num;

	if (orbit < 0) {
		_curOrbit = -1;

		if (_explored) {
			_info = NULL;

			if (oldinfo) {
				oldinfo->discard();
			}
		}

		return;
	}

	planet_id = _game->getOrbitingPlanetID(_star, orbit);

	if (planet_id < 0) {
		throw std::runtime_error("Selected orbit is empty");
	}

	pptr = _game->_planets + planet_id;
	newinfo = new TextLayout;
	newinfo->setFont(FONTSIZE_SMALL, FONT_COLOR_STARWIDGET_PLANET_INFO);
	lineheight = newinfo->lineHeight();

	if (pptr->type == PlanetType::ASTEROIDS) {
		str = gameLang->hstrings(HSTR_STARSYSTEM_ASTEROIDS_INFO);
		newinfo->appendText(str, 0, 0, maxwidth);
		_curOrbit = -1;
	} else if (pptr->type == PlanetType::GAS_GIANT) {
		str = gameLang->hstrings(HSTR_STARSYSTEM_GAS_GIANT_INFO);
		newinfo->appendText(str, 0, 0, maxwidth);
		_reticle->setVariant(MAX_PLAYERS);
		_curOrbit = orbit;
	} else {
		if (pptr->colony >= 0) {
			cptr = _game->_colonies + pptr->colony;
			i = _game->_players[cptr->owner].color;
			_reticle->setVariant(i);
		} else {
			_reticle->setVariant(MAX_PLAYERS);
		}

		num.roman(sptr->planetSeq(pptr->orbit) + 1);
		buf.printf("%s %s", sptr->name, num.c_str());

		if (cptr) {
			buf.append_printf(" (%s)",
				gameLang->estrings(ESTR_RACENAME_ALKARI +
				_game->_players[cptr->owner].picture));
		}

		newinfo->appendText(buf.c_str(), 0, 0, maxwidth);
		climate = _game->planetClimate(planet_id);
		buf.printf("%s, %s", gameLang->estrings(sizesMap[pptr->size]),
			gameLang->estrings(planetClimateMap[climate]));
		newinfo->appendText(buf.c_str(), 0, newinfo->height(),
			maxwidth);

		if (cptr) {
			str = gameLang->hstrings(HSTR_COLONY_POPULATION);
			buf.printf(str, cptr->population,
				_game->planetMaxPop(planet_id, _activePlayer));
		} else {
			str = gameLang->hstrings(HSTR_PLANET_POPULATION);
			buf.printf(str, _game->planetMaxPop(planet_id,
				_activePlayer));
		}

		newinfo->appendText(buf.c_str(), 0, newinfo->height(),
			maxwidth);
		buf.printf("%s, %s",
			gameLang->estrings(mineralsMap[pptr->minerals]),
			gameLang->estrings(ESTR_LGRAVITY_LOW + pptr->gravity));
		newinfo->appendText(buf.c_str(), 0, newinfo->height(),
			maxwidth);
		y = newinfo->height() + 5;

		for (i = 0; cptr && defensiveBuildings[i]; i++) {
			if (!cptr->buildings[defensiveBuildings[i]]) {
				continue;
			}

			str = gameLang->techname(TNAME_BUILDING_NONE +
				defensiveBuildings[i]);
			newinfo->appendText(str, 0, y, maxwidth);
			y += lineheight;
		}

		if (pptr->special && pptr->special < ORION_SPECIAL) {
			str = gameLang->estrings(ESTR_SPECIAL_NONE +
				pptr->special);
			newinfo->appendText(str, 0, y, maxwidth);
		}

		_curOrbit = orbit;
	}

	_reticle->stopAnimation();
	_reticle->startAnimation();
	_info = newinfo;

	if (oldinfo) {
		oldinfo->discard();
	}
}

void StarSystemWidget::redrawPlanets(const Star *sptr, const int *drawlist,
	int x, int y, unsigned curtick) {
	unsigned i, idx, climate, color, offset;
	int px, py;
	double cycle;
	const Image *pimg, *shadow;
	const Planet *pptr;
	const Colony *cptr = NULL;

	for (i = 0; i < MAX_ORBITS; i++) {
		if (sptr->planetIndex[i] < 0) {
			continue;
		}

		pptr = _game->_planets + sptr->planetIndex[i];

		if (pptr->type != PlanetType::ASTEROIDS) {
			_orbits->drawCentered(x, y, i);
		}
	}

	for (i = 0; i < MAX_ORBITS && drawlist[i] >= 0; i++) {
		idx = drawlist[i];
		pptr = _game->_planets + sptr->planetIndex[idx];

		if (pptr->type == PlanetType::ASTEROIDS) {
			_asteroids->drawCentered(x, y, idx);
			continue;
		}

		cptr = NULL;
		px = x + orbitX(idx);
		py = y + orbitY(idx);

		if (pptr->type == PlanetType::GAS_GIANT) {
			offset = PLANET_SIZE_COUNT;
			pimg = (const Image*)_gasGiant;
			shadow = (const Image*)_planetShadow[offset - 1];
			px -= planetSpriteSizes[offset] / 2;
			py -= planetSpriteSizes[offset] / 2;
			offset = planetShadowOffsets[PLANET_CLIMATE_COUNT];
		} else {
			climate = _game->planetClimate(sptr->planetIndex[idx]);
			pimg = (const Image*)_planetImg[climate][pptr->size];
			shadow = (const Image*)_planetShadow[pptr->size];
			px -= planetSpriteSizes[pptr->size] / 2;
			py -= planetSpriteSizes[pptr->size] / 2;
			offset = planetShadowOffsets[climate];

			if (pptr->colony >= 0) {
				cptr = _game->_colonies + pptr->colony;
			}
		}

		pimg->draw(px, py, loopFrame(curtick - _startTick,
			PLANET_ANIM_SPEED, pimg->frameCount()));
		cycle = orbitAngle(idx) / (2 * M_PI);
		cycle -= (int)cycle;
		cycle = cycle < 0 ? cycle + 1 : cycle;
		shadow->draw(px + offset, py + offset,
			(unsigned)(cycle * shadow->frameCount()));
	}

	for (i = 0; i < MAX_ORBITS && drawlist[i] >= 0; i++) {
		idx = drawlist[i];
		pptr = _game->_planets + sptr->planetIndex[idx];

		if (pptr->type != PlanetType::HABITABLE || pptr->colony < 0) {
			continue;
		}

		px = x + orbitX(idx) - planetSpriteSizes[pptr->size] / 2;
		py = y + orbitY(idx) - planetSpriteSizes[pptr->size] / 2;
		cptr = _game->_colonies + pptr->colony;
		color = _game->_players[cptr->owner].color;
		_colonyIcons[color]->draw(px - 6, py);
	}

	if (_curOrbit >= 0) {
		px = x + orbitX(_curOrbit);
		py = y + orbitY(_curOrbit);
		_reticle->redraw(px, py, curtick);
	}
}

void StarSystemWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, w = width(), h = height();
	int drawx = x + getX(), drawy = y + getY(), px, py;
	TextLayout *info;
	const Star *sptr;
	int drawlist[MAX_ORBITS];

	if (!_startTick) {
		_startTick = curtick;
	}

	if (_bg) {
		_bg->draw(drawx, drawy);
	}

	if (!_explored) {
		info = _info;

		if (info) {
			drawy += (h - info->height()) / 2;
			info->redraw(drawx + 16, drawy, curtick);
		}

		return;
	}

	sptr = _game->_starSystems + _star;

	for (i = 0; i < MAX_ORBITS; i++) {
		drawlist[i] = _drawlist[i];
	}

	drawx += w / 2;
	drawy += h / 2;
	_starImg[sptr->spectralClass]->drawCentered(drawx, drawy);

	redrawPlanets(sptr, drawlist, drawx, drawy, curtick);
	// TODO: draw fleets and satellites

	if (sptr->wormhole >= 0) {
		StringBuffer buf;
		const char *str;
		Font *fnt = gameFonts->getFont(FONTSIZE_SMALLER);

		str = gameLang->hstrings(HSTR_STARSYSTEM_WORMHOLE_INFO);
		buf.printf(str, _game->_starSystems[sptr->wormhole].name);
		px = x + getX() + width() - fnt->textWidth(buf.c_str()) - 5;
		py = y + getY() + height() - fnt->height() - 5;
		fnt->renderText(px, py, FONT_COLOR_STARWIDGET_WORMHOLE,
			buf.c_str());
	}

	info = _info;

	if (info) {
		x += getX();
		y += getY();
		gameScreen->fillTransparentRect(x + 13, y + 12,
			info->width() + 7, info->height() + 2, RGBA(0x0, 0x80));
		info->redraw(x + 15, y + 15, curtick);
		drawFrame(x + 11, y + 10, info->width() + 11,
			info->height() + 6, planetInfoFrame);
	}
}

void StarSystemWidget::handleMouseMove(int x, int y, unsigned buttons) {
	int idx, oldHL = _curOrbit;

	idx = findPlanet(x - getX(), y - getY());

	if (idx >= 0 && idx != oldHL) {
		highlightOrbit(idx);
		_onPlanetHighlight(x, y);
	}
}

void StarSystemWidget::handleMouseUp(int x, int y, unsigned button) {
	int idx;

	idx = findPlanet(x - getX(), y - getY());

	if (idx >= 0) {
		_onPlanetSelect(x, y);
	}
}

GalaxyView::GalaxyView(GameState *game) : _game(game), _zoom(0), _zoomX(0),
	_zoomY(0), _startTick(0), _selTick(0), _curStar(-1), _activePlayer(-1),
	_curFleet(NULL) {

	uint8_t tpal[PALSIZE];
	const uint8_t *pal;
	const Player *plr;
	int i, j, k;

	for (i = 0; i < GALAXY_ZOOM_LEVELS; i++) {
		if (galaxySizeFactors[i] == game->_galaxy.sizeFactor) {
			_zoom = i;
			break;
		}
	}

	if (i >= GALAXY_ZOOM_LEVELS) {
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

	w = createWidget(20, 20, 507, 401);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickGalaxyMap));
	w->setMouseMoveCallback(GuiMethod(*this, &GalaxyView::touchGalaxyMap));
	w->setMouseOutCallback(GuiMethod(*this, &GalaxyView::leaveGalaxyMap));

	w = createWidget(249, 5, GALAXY_ARCHIVE, ASSET_GALAXY_GAME_BUTTON,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickGameMenu));
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

	w = createWidget(244, 428, GALAXY_ARCHIVE, ASSET_GALAXY_ZOOMIN_BUTTON,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickZoomInButton));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_ZOOM_BUTTON));

	w = createWidget(244, 455, GALAXY_ARCHIVE, ASSET_GALAXY_ZOOMOUT_BUTTON,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickZoomOutButton));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_ZOOM_BUTTON));

	w = createWidget(544, 441, GALAXY_ARCHIVE, ASSET_GALAXY_TURN_BUTTON,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickTurnButton));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_TURN_BUTTON));

	w = createWidget(547, 52, 65, 67);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickTreasuryInfo));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_TREASURY_INFO));

	w = createWidget(547, 124, 65, 68);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickFleetInfo));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_FLEET_INFO));

	w = createWidget(547, 199, 65, 68);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_FOOD_INFO));

	w = createWidget(547, 273, 65, 67);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickFreighterInfo));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_FREIGHTER_INFO));

	w = createWidget(547, 347, 65, 67);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &GalaxyView::clickResearchInfo));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_RESEARCH_INFO));

	w = createWidget(549, 27, 63, 13);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &GalaxyView::showHelp,
		HELP_GALAXY_STARDATE_BOX));

	if (_game->_gameConfig.multiplayer != MultiplayerType::Single) {
		w = createWidget(73, 5, 88, 14);
		w->setMouseUpCallback(MBUTTON_RIGHT,
			GuiMethod(*this, &GalaxyView::showHelp,
			HELP_GALAXY_MULTIPLAYER_TURN_LIGHTS));
	}
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
	unsigned color = f->getColor();

	return (const Image*)_fleetimg[color][_zoom];
}

const Image *GalaxyView::getStarSprite(const Star *s) const {
	if (s->spectralClass >= SpectralClass::BlackHole) {
		return (const Image*)_bholeimg[_zoom];
	}

	return (const Image*)_starimg[s->spectralClass][_zoom + s->size];
}

int GalaxyView::touchesFleet(unsigned x, unsigned y, const Fleet *f) const {
	int fx, fy;
	const Image *img;

	img = getFleetSprite(f);
	fx = transformFleetX(f);
	fy = transformFleetY(f);

	return isInRect(x, y, fx, fy, img->width(), img->height());
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

	if (_activePlayer >= 0) {
		_game->setActivePlayer(_activePlayer);
	}
}

void GalaxyView::findObject(unsigned x, unsigned y, int *rstar,
	Fleet **rfleet) {

	unsigned i, sx, sy;
	const Image *img;
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

		img = getStarSprite(ptr);
		sx = transformX(ptr->x) - img->width() / 2;
		sy = transformY(ptr->y) - img->height() / 2;

		if (isInRect(x, y, sx, sy, img->width(), img->height())) {
			*rstar = i;
			return;
		}
	}
}
void GalaxyView::clickGalaxyMap(int x, int y, int arg) {
	int star = -1;
	Fleet *f = NULL;

	findObject(x, y, &star, &f);

	if (star >= 0) {
		new MessageBoxWindow(this, "Not implemented yet");
		return;
	}

	if (f) {
		new MessageBoxWindow(this, "Not implemented yet");
		return;
	}
}

void GalaxyView::touchGalaxyMap(int x, int y, int arg) {
	int star = -1;
	Fleet *f = NULL;

	findObject(x, y, &star, &f);

	if (_curStar != star || _curFleet != f) {
		_selTick = 0;
	}

	_curStar = star;
	_curFleet = f;
}

void GalaxyView::leaveGalaxyMap(int x, int y, int arg) {
	_curStar = -1;
	_curFleet = NULL;
}

void GalaxyView::drawStar(const Star *s, Font *fnt, unsigned curtick) {
	int x, y, xoff, idx;
	unsigned i, owner, color, width, tmp, step, total = 0, frame = 0;
	StarKnowledge explored;
	const Image *img;
	const Planet *pptr;
	const char *str;
	unsigned colony_count[MAX_PLAYERS] = {0};
	char buf[STARS_NAME_SIZE + 2];

	x = transformX(s->x);
	y = transformY(s->y);
	img = getStarSprite(s);
	y -= img->height() / 2;
	img->draw(x - img->width() / 2, y);
	y += img->height();

	for (i = 0; i < MAX_ORBITS; i++) {
		if (s->planetIndex[i] < 0) {
			continue;
		}

		pptr = _game->_planets + s->planetIndex[i];

		if (pptr->colony >= 0) {
			owner = _game->_colonies[pptr->colony].owner;
			idx = _activePlayer;

			if (!_game->_players[idx].isPlayerVisible(owner)) {
				continue;
			}

			colony_count[owner]++;
			total++;
		}
	}

	if (_curStar >= 0 && s == _game->_starSystems + _curStar) {
		frame = loopFrame(curtick - _selTick, STAR_ANIM_SPEED,
			GALAXY_ANIM_LENGTH);
	}

	explored = _game->isStarExplored(s, _activePlayer);

	if (!explored) {
		return;
	} else if (explored == STAR_CHARTED) {
		sprintf(buf, "(%s)", s->name);
		str = buf;
	} else {
		str = s->name;
	}

	if (!total) {
		color = FONT_COLOR_STAR_NEUTRAL1 + galaxy_fontanim[frame];
		fnt->centerText(x, y, color, str, OUTLINE_FULL);
		return;
	}

	width = fnt->textWidth(str);
	x -= width / 2;
	xoff = -1;

	try {
		for (i = 0, step = 0; i < _game->_playerCount; i++) {
			if (!colony_count[i]) {
				continue;
			}

			color = galaxyStarColors[_game->_players[i].color];
			color += galaxy_fontanim[frame];
			step += colony_count[i];
			tmp = (step * width) / total;

			gameScreen->setClipRegion(x + xoff, y - 1,
				tmp + 1 - xoff, fnt->height() + 2);
			fnt->renderText(x, y, color, str, OUTLINE_FULL);
			xoff = tmp;
		}
	} catch (...) {
		gameScreen->unsetClipRegion();
		throw;
	}

	gameScreen->unsetClipRegion();
}

void GalaxyView::drawFleet(const Fleet *f, unsigned curtick) {
	unsigned frame = 0;
	const Image *img;

	img = getFleetSprite(f);

	if (f == _curFleet) {
		frame = bounceFrame(curtick - _selTick, STAR_ANIM_SPEED,
			img->frameCount());
	}

	img->draw(transformFleetX(f), transformFleetY(f), frame);
}

void GalaxyView::redrawSidebar(unsigned curtick) {
	unsigned stardate, color, negcolor, x, len;
	ssize_t pos;
	Font *fnt;
	const Player *ptr;
	const char *str;
	StringBuffer buf;

	if (_activePlayer < 0) {
		return;
	}

	// FIXME: Use strings from game data
	ptr = _game->_players + _activePlayer;
	fnt = gameFonts->getFont(FONTSIZE_SMALL);
	negcolor = loopFrame(curtick - _startTick, GALAXY_ANIM_SPEED,
		GALAXY_ANIM_LENGTH);
	negcolor = FONT_COLOR_DEFICIT1 + galaxy_fontanim[negcolor];
	stardate = _game->_gameConfig.stardate;
	buf.printf("%d.%d", stardate / 10, stardate % 10);
	fnt->centerText(GALAXY_SIDEBAR_X, 29, FONT_COLOR_GALAXY_GUI,
		buf.c_str(), OUTLINE_FULL);

	// Treasury and income
	buf.printf("%d BC", ptr->BC);
	color = ptr->BC < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 96, color, buf.c_str(), OUTLINE_FULL);
	buf.printf("%+d BC", ptr->surplusBC);
	color = ptr->surplusBC < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 108, color, buf.c_str(),
		OUTLINE_FULL);

	// Fleet command points
	buf.printf("%+d (%u)", ptr->commandPoints - ptr->usedCommandPoints,
		ptr->commandPoints);
	str = buf.c_str();
	len = fnt->textWidth(str);
	pos = buf.find('(');
	str += pos;
	x = GALAXY_SIDEBAR_X - len / 2;
	color = FONT_COLOR_GALAXY_GUI;

	if (ptr->usedCommandPoints > (int)ptr->commandPoints) {
		color = negcolor;
	}

	fnt->renderText(x + len + 1 - fnt->textWidth(str), 181,
		FONT_COLOR_GALAXY_GUI, str, OUTLINE_FULL);
	buf.truncate(pos);
	fnt->renderText(x, 181, color, buf.c_str(), OUTLINE_FULL);

	// Food surplus
	buf.printf("%+d", ptr->surplusFood);
	color = ptr->surplusFood < 0 ? negcolor : FONT_COLOR_GALAXY_GUI;
	fnt->centerText(GALAXY_SIDEBAR_X, 255, color, buf.c_str(),
		OUTLINE_FULL);

	// Freighters status
	buf.printf("%+d (%u)", ptr->surplusFreighters, ptr->totalFreighters);
	str = buf.c_str();
	len = fnt->textWidth(str);
	pos = buf.find('(');
	str += pos;
	x = GALAXY_SIDEBAR_X - len / 2;
	color = (ptr->surplusFreighters < 0) ? negcolor : FONT_COLOR_GALAXY_GUI;

	fnt->renderText(x + len + 1 - fnt->textWidth(str), 330,
		FONT_COLOR_GALAXY_GUI, str, OUTLINE_FULL);
	buf.truncate(pos);
	fnt->renderText(x, 330, color, buf.c_str(), OUTLINE_FULL);

	// Research progress
	if (ptr->researchItem) {
		// FIXME: draw expected time to completion
		//buf.printf("~%d turns", turns_remaining);
		//fnt->centerText(GALAXY_SIDEBAR_X, 391, FONT_COLOR_GALAXY_GUI,
		//	buf.c_str(), OUTLINE_FULL);
		buf.printf("%+'d RP", ptr->researchProduced);
		fnt->centerText(GALAXY_SIDEBAR_X, 403, FONT_COLOR_GALAXY_GUI,
			buf.c_str(), OUTLINE_FULL);
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
	unsigned i, frame, bhshift = 0;
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

	if (!_selTick) {
		_selTick = curtick;
	}

	fnt = gameFonts->getFont(font_sizes[_zoom]);

	gameScreen->clear();
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
		gameScreen->drawLine(transformX(s1->x), transformY(s1->y),
			transformX(s2->x), transformY(s2->y), 36, 36, 40);
	}

	// Draw stars and black holes
	for (i = 0; i < _game->_starSystemCount; i++) {
		Star *ptr = _game->_starSystems + i;

		if (ptr->spectralClass < SpectralClass::BlackHole) {
			drawStar(ptr, fnt, curtick);
		} else {
			x = transformX(ptr->x);
			y = transformY(ptr->y);
			img = getStarSprite(ptr);

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
			drawFleet(fnode->data, curtick);
		}

		fnode = ptr->getLeavingFleets();

		if (fnode && fnode->data) {
			drawFleet(fnode->data, curtick);
		}
	}

	for (fnode = _game->getMovingFleets(); fnode; fnode = fnode->next()) {
		if (!fnode->data) {
			continue;
		}

		drawFleet(fnode->data, curtick);
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
		frame = loopFrame(curtick - _startTick, GALAXY_ANIM_SPEED,
			img->frameCount());
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
	gui_stack->push(new ColoniesListView(_game, _activePlayer));
}

void GalaxyView::clickPlanetsButton(int x, int y, int arg) {
	gui_stack->push(new PlanetsListView(_game, _activePlayer));
}

void GalaxyView::clickFleetsButton(int x, int y, int arg) {
	gui_stack->push(new FleetListView(_game, _activePlayer));
}

void GalaxyView::clickLeadersButton(int x, int y, int arg) {
	gui_stack->push(new LeaderListView(_game, _activePlayer));
}

void GalaxyView::clickRacesButton(int x, int y, int arg) STUB(this)

void GalaxyView::clickInfoButton(int x, int y, int arg) STUB(this)

void GalaxyView::clickZoomInButton(int x, int y, int arg) STUB(this)

void GalaxyView::clickZoomOutButton(int x, int y, int arg) STUB(this)

void GalaxyView::clickTurnButton(int x, int y, int arg) STUB(this)

void GalaxyView::clickTreasuryInfo(int x, int y, int arg) STUB(this)

void GalaxyView::clickFleetInfo(int x, int y, int arg) STUB(this)

void GalaxyView::clickFreighterInfo(int x, int y, int arg) STUB(this)

void GalaxyView::clickResearchInfo(int x, int y, int arg) {
	new ResearchSelectWindow(this, _game, _activePlayer, 1);
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

		flag_id = ptr->picture * MAX_PLAYERS + ptr->color;
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
	const char *player_fmt;
	const Player *ptr;
	Font *fnt;
	StringBuffer buf;
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

	player_fmt = gameLang->hstrings(HSTR_PLAYER_SELECT_FMT);
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
		// FIXME: Find and use real player score
		buf.printf(player_fmt, ptr->name, ptr->race, "0");

		if (_game->_players[_humans[i]].playerDoneFlags) {
			color = 1;
		} else if (int(i) == _currentPlayer) {
			color = loopFrame(curtick - _animStart, 30,
				PSELECT_ANIM_LENGTH);
			color = fontanim[color];
		} else {
			color = 3;
		}

		color += color_list[ptr->color];
		fnt->renderText(x + 116, y + i * h - 9, color, buf.c_str());

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

PlanetsListView::PlanetsListView(GameState *game, int activePlayer) :
	_game(game), _minimap(NULL), _scroll(NULL), _sortChoice(NULL),
	_enemyFilter(NULL), _gravityFilter(NULL), _envFilter(NULL),
	_mineralFilter(NULL), _rangeFilter(NULL), _colonyToggle(NULL),
	_outpostToggle(NULL), _scrollgrab(0), _curslot(-1),
	_activePlayer(activePlayer), _planetCount(0) {

	unsigned i, j, k, color;
	int dest, *shiplist;
	ToggleWidget *shipToggle;
	const uint8_t *pal;

	for (i = 0; i < MAX_PLANETS; i++) {
		_colonyShips[i] = _outpostShips[i] = -1;
	}

	_bg = gameAssets->getImage(PLANET_ARCHIVE, ASSET_PLANETLIST_BG);
	pal = _bg->palette();
	color = _game->_players[_activePlayer].color;
	_shipimg = gameAssets->getImage(PLANET_ARCHIVE,
		ASSET_PLANETLIST_SHIP_IMAGES + color, pal);

	for (i = 0, k = 0; i < PLANET_CLIMATE_COUNT; i++) {
		for (j = 0; j < PLANET_SIZE_COUNT; j++, k++) {
			_planetimg[i][j] = gameAssets->getImage(PLANET_ARCHIVE,
				ASSET_PLANETLIST_PLANET_IMAGES + k, pal);
		}
	}

	initWidgets();
	changeFilter(0, 0, 0);

	for (i = 0; i < _game->_shipCount; i++) {
		const Ship *sptr = _game->_ships + i;

		if (!sptr->isActive()) {
			continue;
		}

		if (sptr->design.type == ShipType::COLONY_SHIP) {
			shiplist = _colonyShips;
			shipToggle = _colonyToggle;
		} else if (sptr->design.type == ShipType::OUTPOST_SHIP) {
			shiplist = _outpostShips;
			shipToggle = _outpostToggle;
		} else {
			continue;
		}

		// Destination planet ID for colony and outpost ships
		dest = sptr->design.weapons[0].type;

		if ((sptr->status == ShipState::LeavingOrbit ||
			sptr->status == ShipState::InTransit) && dest >= 0) {
			shiplist[dest] = i;
		} else {
			shipToggle->disable(0);
		}
	}
}

void PlanetsListView::initWidgets(void) {
	unsigned i;
	const uint8_t *pal = _bg->palette();
	Widget *w = NULL;

	for (i = 0; i < PLANET_LIST_ROWS; i++) {
		w = createWidget(18, PLANET_LIST_FIRST_ROW +
			i * PLANET_LIST_ROW_DIST, 398, PLANET_LIST_ROW_HEIGHT);
		w->setMouseOverCallback(GuiMethod(*this,
			&PlanetsListView::highlightSlot, i));
		w->setMouseMoveCallback(GuiMethod(*this,
			&PlanetsListView::highlightSlot, i));
		w->setMouseOutCallback(GuiMethod(*this,
			&PlanetsListView::highlightSlot, -1));
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &PlanetsListView::clickSlot, i));
		w->setMouseUpCallback(MBUTTON_RIGHT,
			GuiMethod(*this, &PlanetsListView::showHelp,
			HELP_PLANETLIST_SLOT));
	}

	_minimap = new StarmapWidget(443, 17, 180, 116, _game, _activePlayer,
		PLANET_ARCHIVE, ASSET_PLANETLIST_STAR_IMAGES, pal);
	addWidget(_minimap);
	_minimap->setStarSelectCallback(GuiMethod(*this,
		&PlanetsListView::handleSelectStar));
	_minimap->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_MINIMAP));


	_scroll = new ScrollBarWidget(423, 38, PLANET_LIST_SCROLL_WIDTH, 407,
		8, 8, planetListScrollTexture);
	addWidget(_scroll);
	_scroll->setBeginScrollCallback(GuiMethod(*this,
		&PlanetsListView::handleBeginScroll));
	_scroll->setEndScrollCallback(GuiMethod(*this,
		&PlanetsListView::handleEndScroll));
	_scroll->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_SCROLLBAR));

	w = createWidget(422, 16, PLANET_ARCHIVE,
		ASSET_PLANETLIST_SCROLL_UP_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollMinus));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_SCROLLBAR));

	w = createWidget(421, 448, PLANET_ARCHIVE,
		ASSET_PLANETLIST_SCROLL_DOWN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollPlus));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_SCROLLBAR));

	_sortChoice = new ChoiceWidget(441, 200, 183, 24, 3);
	addWidget(_sortChoice);
	_sortChoice->setChoiceButton(0, 0, 0, 60, 24, PLANET_ARCHIVE,
		ASSET_PLANETLIST_CLIMATE_SORT, pal, 1);
	_sortChoice->setChoiceButton(1, 60, 0, 66, 24, PLANET_ARCHIVE,
		ASSET_PLANETLIST_MINERALS_SORT, pal, 1);
	_sortChoice->setChoiceButton(2, 126, 0, 57, 24, PLANET_ARCHIVE,
		ASSET_PLANETLIST_SIZE_SORT, pal, 1);
	_sortChoice->setValueChangeCallback(GuiMethod(*this,
		&PlanetsListView::changeSort));
	_sortChoice->button(0)->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_CLIMATE_BUTTON));
	_sortChoice->button(1)->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_MINERALS_BUTTON));
	_sortChoice->button(2)->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_SIZE_BUTTON));

	_enemyFilter = new ToggleWidget(441, 266, 183, 21, PLANET_ARCHIVE,
		ASSET_PLANETLIST_ENEMY_TOGGLE, pal);
	addWidget(_enemyFilter);
	_enemyFilter->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::changeFilter));
	_enemyFilter->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_ENEMY_TOGGLE));

	_gravityFilter = new ToggleWidget(441, 289, 183, 22, PLANET_ARCHIVE,
		ASSET_PLANETLIST_GRAVITY_TOGGLE, pal);
	addWidget(_gravityFilter);
	_gravityFilter->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::changeFilter));
	_gravityFilter->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_GRAVITY_TOGGLE));

	_envFilter = new ToggleWidget(441, 312, 183, 22, PLANET_ARCHIVE,
		ASSET_PLANETLIST_ENV_TOGGLE, pal);
	addWidget(_envFilter);
	_envFilter->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::changeFilter));
	_envFilter->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_ENV_TOGGLE));

	_mineralFilter = new ToggleWidget(441, 335, 183, 22, PLANET_ARCHIVE,
		ASSET_PLANETLIST_MINERAL_TOGGLE, pal);
	addWidget(_mineralFilter);
	_mineralFilter->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::changeFilter));
	_mineralFilter->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_MINERAL_TOGGLE));

	// FIXME: Implement and enable by default
	_rangeFilter = new ToggleWidget(441, 358, 183, 20, PLANET_ARCHIVE,
		ASSET_PLANETLIST_RANGE_TOGGLE, pal);
	addWidget(_rangeFilter);
	_rangeFilter->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::changeFilter));
	_rangeFilter->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_RANGE_TOGGLE));

	_colonyToggle = new ToggleWidget(454, 386, 156, 23, PLANET_ARCHIVE,
		ASSET_PLANETLIST_COLONY_TOGGLE, pal, 1, 2);
	addWidget(_colonyToggle);
	_colonyToggle->setDisabledSprite(PLANET_ARCHIVE,
		ASSET_PLANETLIST_COLONY_TOGGLE, pal, 0);
	_colonyToggle->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::clickColonyToggle));
	_colonyToggle->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_COLONY_TOGGLE));
	_colonyToggle->disable(1);

	_outpostToggle = new ToggleWidget(454, 413, 157, 25, PLANET_ARCHIVE,
		ASSET_PLANETLIST_OUTPOST_TOGGLE, pal, 1, 2);
	addWidget(_outpostToggle);
	_outpostToggle->setDisabledSprite(PLANET_ARCHIVE,
		ASSET_PLANETLIST_OUTPOST_TOGGLE, pal, 0);
	_outpostToggle->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::clickOutpostToggle));
	_outpostToggle->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_OUTPOST_TOGGLE));
	_outpostToggle->disable(1);

	w = createWidget(454, 440, PLANET_ARCHIVE,
		ASSET_PLANETLIST_RETURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &PlanetsListView::clickReturn));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_RETURN_BUTTON));

	w = createWidget(18, 15, 397, 16);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_HEADER));

	w = createWidget(441, 143, 182, 22);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &PlanetsListView::showHelp,
		HELP_PLANETLIST_MINIMAP));
}

void PlanetsListView::handleBeginScroll(int x, int y, int arg) {
	_scrollgrab = 1;
}

void PlanetsListView::handleEndScroll(int x, int y, int arg) {
	_scrollgrab = 0;
	selectCurrentWidget(x, y, 0);
}

void PlanetsListView::handleSelectStar(int x, int y, int arg) {
	unsigned i;
	int star;
	const Planet *ptr;
	const Star *sptr;
	const char *str;

	star = _minimap->selectedStar();

	if (star < 0) {
		return;
	}

	sptr = _game->_starSystems + star;

	if (sptr->spectralClass == SpectralClass::BlackHole) {
		return;
	}

	// Jump to the first planet in list orbiting the selected star
	for (i = 0; i < _planetCount; i++) {
		ptr = _game->_planets + _planets[i];

		if (ptr->star == (unsigned)star) {
			_scroll->setPosition(i);
			return;
		}
	}

	// No planet found
	StringBuffer buf;

	// Note: the original game did not check whether the star is explored
	if (_game->isStarExplored(sptr, _activePlayer)) {
		str = sptr->name;
	} else {
		str = gameLang->hstrings(HSTR_STAR_UNEXPLORED);
	}

	buf.printf(gameLang->hstrings(HSTR_PLANETLIST_NO_PLANETS), str);
	new ErrorWindow(this, buf.c_str());
}

void PlanetsListView::redraw(unsigned curtick) {
	Font *fnt, *smallFnt;
	unsigned i, y, climate, color, offset = _scroll->position();
	int simpleY, fullY, smallY, owner, penalty, curstar;
	const char *str, *foodstr, *prodstr, *popstr, *penaltystr;
	StringBuffer buf, num;
	const Image *img;
	const Planet *ptr;
	const Star *sptr;
	const Player *player = _game->_players + _activePlayer;
	const BilistNode<Fleet> *node;

	fnt = gameFonts->getFont(FONTSIZE_SMALL);
	smallFnt = gameFonts->getFont(FONTSIZE_SMALLER);
	simpleY = (PLANET_LIST_ROW_HEIGHT - fnt->height()) / 2;
	fullY = PLANET_LIST_ROW_HEIGHT - 4 - fnt->height() - smallFnt->height();
	fullY /= 2;
	smallY = fullY + fnt->height() + 4;
	foodstr = gameLang->hstrings(HSTR_PLANET_FOOD);
	prodstr = gameLang->hstrings(HSTR_PLANET_PRODUCTION);
	popstr = gameLang->hstrings(HSTR_PLANET_POPULATION);
	penaltystr = gameLang->hstrings(HSTR_PLANET_PRODUCTION_PENALTY);

	gameScreen->clear();
	_bg->draw(0, 0);

	for (i = 0; i < PLANET_LIST_ROWS && offset + i < _planetCount; i++) {
		ptr = _game->_planets + _planets[offset + i];
		sptr = _game->_starSystems + ptr->star;
		y = PLANET_LIST_FIRST_ROW + i * PLANET_LIST_ROW_DIST;
		color = MAX_PLAYERS;
		owner = -1;
		climate = _game->planetClimate(_planets[offset + i]);

		if (ptr->colony >= 0) {
			owner = _game->_colonies[ptr->colony].owner;
			color = _game->_players[owner].color;
		}

		if (_curslot == (int)i) {
			color = planetListFontColorsBright[color];
		} else {
			color = planetListFontColors[color];
		}

		// Planet name and image
		img = (const Image*)_planetimg[climate][ptr->size];
		img->draw(61 - img->width() / 2, y + 21 - img->height() / 2);
		num.roman(sptr->planetSeq(ptr->orbit) + 1);
		buf.printf("%s %s", sptr->name, num.c_str());
		fnt->centerText(61, y + 29, color, buf.c_str());

		if (ptr->special && ptr->special < ORION_SPECIAL) {
			str = gameLang->estrings(ESTR_SPECIAL_NONE +
				ptr->special);
			smallFnt->centerText(61, y + 1, color, str);
		}

		if (owner >= 0) {
			str = gameLang->estrings(ESTR_RACENAME_ALKARI +
				_game->_players[owner].picture);
			buf.printf("(%s)", str);
			smallFnt->centerText(61, y + 41, color, buf.c_str());
		} else {
			// FIXME: Check fleet visibility
			node = sptr->getOrbitingFleets();

			for (; node; node = node->next()) {
				if (!node->data) {
					continue;
				}

				if (node->data->getOwner() > MAX_PLAYERS) {
					break;
				}
			}

			if (node) {
				buf.printf("(%s)", node->data->getRace());
				smallFnt->centerText(61, y + 41, color,
					buf.c_str());
			}
		}

		if (_outpostShips[_planets[offset + i]] >= 0) {
			_shipimg->draw(PLANET_LIST_OUTPOST_X,
				y + PLANET_LIST_SHIP_YOFF);
		}

		if (_colonyShips[_planets[offset + i]] >= 0) {
			_shipimg->draw(PLANET_LIST_COLONY_X,
				y + PLANET_LIST_SHIP_YOFF);
		}

		// Planet climate
		str = gameLang->estrings(planetClimateMap[climate]);
		fnt->centerText(138, y + fullY, color, str);
		buf.printf(foodstr, (int)ptr->foodbase);
		smallFnt->centerText(138, y + smallY, color, buf.c_str());

		// Planet Gravity
		str = gameLang->estrings(ESTR_LGRAVITY_LOW + ptr->gravity);
		penalty = player->gravityPenalty(ptr->gravity);

		if (penalty) {
			buf.printf(penaltystr, -penalty);
			fnt->centerText(220, y + fullY, color, str);
			smallFnt->centerText(220, y + smallY, color,
				buf.c_str());
		} else {
			fnt->centerText(220, y + simpleY, color, str);
		}

		// Minerals
		str = gameLang->estrings(mineralsMap[ptr->minerals]);
		fnt->centerText(306, y + fullY, color, str);
		buf.printf(prodstr, (int)ptr->baseProduction());
		smallFnt->centerText(306, y + smallY, color, buf.c_str());

		// Planet size
		str = gameLang->estrings(sizesMap[ptr->size]);
		fnt->centerText(385, y + fullY, color, str);
		buf.printf(popstr, _game->planetMaxPop(_planets[offset + i],
			_activePlayer));
		smallFnt->centerText(385, y + smallY, color, buf.c_str());
	}

	curstar = _minimap->highlightedStar();

	if (curstar >= 0) {
		sptr = _game->_starSystems + curstar;

		if (sptr->spectralClass == SpectralClass::BlackHole) {
			color = FONT_COLOR_PLAYER_RED4;
			str = gameLang->hstrings(HSTR_BLACK_HOLE);
		} else if (_game->isStarExplored(sptr,
			_activePlayer) >= STAR_CHARTED) {

			color = FONT_COLOR_PLANET_LIST_BRIGHT;
			str = sptr->name;
		} else {
			color = FONT_COLOR_PLANET_LIST_BRIGHT;
			str = gameLang->hstrings(HSTR_STAR_UNEXPLORED);
		}

		fnt->centerText(532, 143 + (22 - fnt->height()) / 2, color,
			str);
	}

	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void PlanetsListView::handleMouseMove(int x, int y, unsigned buttons) {
	if (_scrollgrab) {
		_scroll->handleMouseMove(x, y, buttons);
		return;
	}

	GuiView::handleMouseMove(x, y, buttons);
}

void PlanetsListView::handleMouseUp(int x, int y, unsigned button) {
	if (_scrollgrab) {
		_scroll->handleMouseUp(x, y, button);
		return;
	}

	GuiView::handleMouseUp(x, y, button);
}

void PlanetsListView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _bg->palette());
}

void PlanetsListView::highlightSlot(int x, int y, int arg) {
	unsigned pid, sy, w, h, offset = _scroll->position();
	const Planet *ptr;

	if (arg >= 0 && arg < PLANET_LIST_ROWS && offset + arg < _planetCount) {
		_curslot = arg;
		pid = _planets[offset + arg];
		ptr = _game->_planets + pid;
		_minimap->highlightStar(ptr->star);
		sy = PLANET_LIST_FIRST_ROW + arg * PLANET_LIST_ROW_DIST +
			PLANET_LIST_SHIP_YOFF;
		w = _shipimg->width();
		h = _shipimg->height();

		// hide highlight if the mouse is over ship icon
		if ((_colonyShips[pid] >= 0 &&
			isInRect(x, y, PLANET_LIST_COLONY_X, sy, w, h)) ||
			(_outpostShips[pid] >= 0 &&
			isInRect(x, y, PLANET_LIST_OUTPOST_X, sy, w, h))) {
			_curslot = -1;
		}
	} else {
		_curslot = -1;
	}
}

void PlanetsListView::clickSlot(int x, int y, int arg) {
	// FIXME: implement sending and cancelling colony/outpost ships
}

void PlanetsListView::changeFilter(int x, int y, int arg) {
	unsigned i, j, count, climate;
	int f_enemy, f_gravity, f_env, f_mineral, f_range;
	int owner;
	const Planet *ptr;
	const Star *sptr;
	const Player *player = _game->_players + _activePlayer;
	const BilistNode<Fleet> *node;

	f_enemy = _enemyFilter->value();
	f_gravity = _gravityFilter->value();
	f_env = _envFilter->value();
	f_mineral = _mineralFilter->value();
	f_range = _rangeFilter->value();

	if (f_range) {
		_rangeFilter->setValue(0);
		new MessageBoxWindow(this, "Range filter not implemented");
	}

	for (i = 0, count = 0; i < _game->_starSystemCount; i++) {
		sptr = _game->_starSystems + i;

		if (_game->isStarExplored(sptr, _activePlayer) < STAR_CHARTED) {
			continue;
		}

		if (f_enemy) {
			// Hostile colony in this star system
			if (sptr->hasColony & ~(1 << _activePlayer)) {
				continue;
			}

			node = sptr->getOrbitingFleets();

			for (; node; node = node->next()) {
				if (!node->data) {
					continue;
				}

				if (node->data->getOwner() != _activePlayer) {
					break;
				}
			}

			// Hostile fleet orbiting star
			// FIXME: ignore hidden fleets (known but unvisited
			// star)
			if (node) {
				continue;
			}
		}

		// FIXME: Implement range filter

		for (j = 0; j < MAX_ORBITS; j++) {
			if (sptr->planetIndex[j] < 0) {
				continue;
			}

			ptr = _game->_planets + sptr->planetIndex[j];
			owner = -1;

			if (ptr->colony >= 0) {
				owner = _game->_colonies[ptr->colony].owner;
			}

			// Ignore invalid and own planets
			if (ptr->type != PlanetType::HABITABLE ||
				owner == _activePlayer) {
				continue;
			}

			if (f_gravity &&
				player->gravityPenalty(ptr->gravity) < 0) {
				continue;
			}

			climate = _game->planetClimate(sptr->planetIndex[j]);

			if (f_env && climate < PlanetClimate::DESERT) {
				continue;
			}

			if (f_mineral &&
				ptr->minerals < PlanetMinerals::ABUNDANT) {
				continue;
			}

			_planets[count++] = sptr->planetIndex[j];
		}
	}

	_planetCount = count;
	_scroll->setRange(_planetCount);
	changeSort(0, 0, 0);
}

void PlanetsListView::changeSort(int x, int y, int arg) {
	gamestate_cmp_func cmplist[3] = {
		cmpPlanetClimate, cmpPlanetMinerals, cmpPlanetMaxPop
	};

	_game->sort_ids(_planets, _planetCount, _activePlayer,
		cmplist[_sortChoice->value()]);
}

// FIXME: Implement sending colony and outpost ships
void PlanetsListView::clickColonyToggle(int x, int y, int arg) {
	_colonyToggle->setValue(0);
	new MessageBoxWindow(this, "Not implemented yet");
}

void PlanetsListView::clickOutpostToggle(int x, int y, int arg) {
	_outpostToggle->setValue(0);
	new MessageBoxWindow(this, "Not implemented yet");
}

void PlanetsListView::clickReturn(int x, int y, int arg) {
	exitView();
}

ColoniesListView::ColoniesListView(GameState *game, int activePlayer) :
    _game(game), _activePlayer(activePlayer),_scroll(NULL), _selectedSlot(-1) {

	_bg = gameAssets->getImage(COLONY_ARCHIVE, ASSET_COLONYLIST_BG);

	initWidgets();
}

void ColoniesListView::initWidgets(void) {
	unsigned i;
	const uint8_t *pal = _bg->palette();
	Widget *w = NULL;

	for (i = 0; i < COLONY_LIST_ROWS; i++) {
		w = createWidget(10,
				 COLONY_LIST_FIRST_ROW + i * (COLONY_LIST_ROW_HEIGHT + COLONY_LIST_ROW_BOTTOM_PADDING),
				 398,
				 COLONY_LIST_ROW_HEIGHT);
		w->setMouseOverCallback(GuiMethod(*this,
			&ColoniesListView::highlightSlot, i));
		w->setMouseMoveCallback(GuiMethod(*this,
			&ColoniesListView::highlightSlot, i));
		w->setMouseOutCallback(GuiMethod(*this,
			&ColoniesListView::highlightSlot, -1));
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &ColoniesListView::clickSlot, i));
	}

	w = createWidget(531, 445, 156, 25);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &ColoniesListView::clickReturn));
	w->setClickSprite(MBUTTON_LEFT, COLONY_ARCHIVE,
		ASSET_COLONYLIST_RETURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &ColoniesListView::showHelp,
			HELP_RETURN_BUTTON));
}

void ColoniesListView::highlightSlot(int x, int y, int arg) {
	if (arg >= 0 && arg < COLONY_LIST_ROWS && arg < _game->_colonyCount) {
		_curslot = arg;
	} else {
		_curslot = -1;
	}
}

void ColoniesListView::clickSlot(int x, int y, int arg) {
	_selectedSlot = arg;
}

// FIXME: Render the colonies in alphanumeric order maybe
void ColoniesListView::redraw(unsigned curtick) {
	Font *fnt;
	// unsigned i, offset = _scroll->position();
	unsigned i, j, y, color;
	int owner;
	StringBuffer buf, num;
	const Planet *planet_ptr;
	const Star *star_ptr;
	const Colony *colony_ptr;

	fnt = gameFonts->getFont(FONTSIZE_SMALL);

	gameScreen->clear();
	_bg->draw(0, 0);

	unsigned offset = 0; // FIXME: This should come from the scrollbar
	int colony_row = 0;
	for (i = 0; i < _game->_planetCount; i++) {
		y = COLONY_LIST_FIRST_ROW + COLONY_LIST_ROW_DIST + colony_row * (COLONY_LIST_ROW_HEIGHT + COLONY_LIST_ROW_BOTTOM_PADDING);
		planet_ptr = _game->_planets + offset + i;
		if (planet_ptr->colony < 0) continue;

		colony_ptr = &_game->_colonies[planet_ptr->colony];
		if (colony_ptr->owner != _activePlayer) continue;

		star_ptr = &_game->_starSystems[planet_ptr->star];
		if (_curslot == colony_row) {
			color = FONT_COLOR_COLONY_LIST_BRIGHT;
			// Fill the lower right box with the info from the selected colony, if any
			renderPlanetDetail(planet_ptr, colony_ptr);
		} else {
			color = FONT_COLOR_COLONY_LIST;
		}
		num.roman(star_ptr->planetSeq(planet_ptr->orbit) + 1);
		buf.printf("%s %s", star_ptr->name, num.c_str());

		fnt->centerText(COLONY_LIST_ROW_LEFT_PADDING, y, color, buf.c_str());

		drawColonistsJobs(colony_ptr, curtick);

		int empireDetailLeftPosition = 520;
		int empireDetailRightPosition = 625;
		int empireDetailTopPosition = 353;
		int empireDetailRowMargin = 13;
		buf.printf("Reserve:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 0, color, buf.c_str());
		buf.printf("%d", _game->_players[_activePlayer].BC);
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 0, color, buf.c_str());

		buf.printf("Income:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 1, color, buf.c_str());
		if (_game->_players[_activePlayer].BC > 0) {
			buf.printf("+%d", _game->_players[_activePlayer].surplusBC);
		} else {
			buf.printf("-%d", _game->_players[_activePlayer].surplusBC);
		}
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 1, color, buf.c_str());

		buf.printf("Population:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 2, color, buf.c_str());
		buf.printf("%d", _game->_players[_activePlayer].totalPop);
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 2, color, buf.c_str());

		buf.printf("Freighters:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 3, color, buf.c_str());
		buf.printf("%d", _game->_players[_activePlayer].surplusFreighters);
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 3, color, buf.c_str());

		buf.printf("Food:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 4, color, buf.c_str());
		if (_game->_players[_activePlayer].surplusFood >= 0) {
			buf.printf("+%d", _game->_players[_activePlayer].surplusFood);
		} else {
			buf.printf("-%d", _game->_players[_activePlayer].surplusFood);
		}
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 4, color, buf.c_str());

		buf.printf("Research:");
		fnt->renderText(empireDetailLeftPosition, empireDetailTopPosition + empireDetailRowMargin * 5, color, buf.c_str());
		buf.printf("%d", _game->_players[_activePlayer].researchProduced);
		fnt->rightText(empireDetailRightPosition, empireDetailTopPosition + empireDetailRowMargin * 5, color, buf.c_str());

		colony_row++;
	}

	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void ColoniesListView::clickReturn(int x, int y, int arg) {
	exitView();
}

void ColoniesListView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _bg->palette());
}

void ColoniesListView::renderPlanetDetail(const Planet *planet_ptr, const Colony *colony_ptr) {
	StringBuffer buf;
	Font *fnt;

	fnt = gameFonts->getFont(FONTSIZE_SMALLER);

	switch(planet_ptr->size) {
	case TINY_PLANET:
		buf.printf("Tiny");
		break;
	case SMALL_PLANET:
		buf.printf("Small");
		break;
	case MEDIUM_PLANET:
		buf.printf("Medium");
		break;
	case LARGE_PLANET:
		buf.printf("Large");
		break;
	default:
		buf.printf("Huge");
	}

	switch(planet_ptr->climate) {
	case TOXIC:
		buf.append(" Toxic");
		break;
	case RADIATED:
		buf.append(" Radiated");
		break;
	case BARREN:
		buf.append(" Barren");
		break;
	case DESERT:
		buf.append(" Desert");
		break;
	case TUNDRA:
		buf.append(" Tundra");
		break;
	case OCEAN:
		buf.append(" Ocean");
		break;
	case SWAMP:
		buf.append(" Swamp");
		break;
	case ARID:
		buf.append(" Arid");
		break;
	case TERRAN:
		buf.append(" Terran");
		break;
	default:
		buf.append(" Gaia");
	}

	fnt->renderText(COLONY_LIST_DETAILS_LEFT_PADDING, 354, FONT_COLOR_COLONY_LIST, buf.c_str());

	switch(planet_ptr->gravity) {
	case LOW_G:
		buf.printf("Low Gravity");
		break;
	case NORMAL_G:
		buf.printf("Normal Gravity");
		break;
	default:
		buf.printf("Heavy Gravity");
	}
	fnt->renderText(COLONY_LIST_DETAILS_LEFT_PADDING, 365, FONT_COLOR_COLONY_LIST, buf.c_str());

	switch(planet_ptr->minerals) {
	case ULTRA_POOR:
		buf.printf("Mineral Ultra poor");
		break;
	case POOR:
		buf.printf("Mineral Poor");
		break;
	case ABUNDANT:
		buf.printf("Mineral Abundant");
		break;
	case RICH :
		buf.printf("Mineral Rich");
		break;
	default:
		buf.printf("Mineral Ultra rich");
	}
	fnt->renderText(COLONY_LIST_DETAILS_LEFT_PADDING, 376, FONT_COLOR_COLONY_LIST, buf.c_str());

	// FIXME: Max population always seems to be zero
	buf.printf("Population (%d/%d)",
	    colony_ptr->population,
	    _game->planetMaxPop(colony_ptr->planet, _activePlayer));
	fnt->renderText(COLONY_LIST_DETAILS_LEFT_PADDING, 387, FONT_COLOR_COLONY_LIST, buf.c_str());
}

void ColoniesListView::drawColonistsJobs(const Colony* colony_ptr, int curtick) {
	int farmers = 0;
	int workers = 0;
	int scientists = 0;
	int j = 0;

	for (j = 0; j < MAX_POPULATION; j++) {
		if (colony_ptr->colonists[j].job == FARMER) {
			farmers++;
		} else if (colony_ptr->colonists[j].job == WORKER) {
			workers++;
		} else if (colony_ptr->colonists[j].job == SCIENTIST) {
			scientists++;
		}
	}

	farmers = colony_ptr->population - workers + scientists;

	if (farmers > colony_ptr->population) {
		farmers = colony_ptr->population;
	}

	ColonistPickerWidget* f = new ColonistPickerWidget(0, 0, 100, 90, colony_ptr, FARMER, _game->_players, _game->_playerCount);
	f->redraw(103, 38, curtick);

	// FIXME: This is showing no workers on some colonies...
	ColonistPickerWidget* w = new ColonistPickerWidget(0, 0, 100, 90, colony_ptr, WORKER, _game->_players, _game->_playerCount);
	w->redraw(237, 38, curtick);

	ColonistPickerWidget* s = new ColonistPickerWidget(0, 0, 100, 90, colony_ptr, SCIENTIST, _game->_players, _game->_playerCount);
	s->redraw(377, 38, curtick);
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
	w = createWidget(40, 43, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_SAVE, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickSave));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_SAVE));

	// Load Game button
	w = createWidget(147, 43, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_LOAD,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickLoad));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_LOAD));

	// New Game button
	w = createWidget(40, 88, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_NEW, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickNew));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_NEWGAME));

	// Quit Game button
	w = createWidget(147, 88, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_QUIT,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickQuit));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_QUIT));

	// Settings button
	w = createWidget(40, 307, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_SETTINGS,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &MainMenuWindow::clickSettings));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_SETTINGS));

	// Return button
	w = createWidget(151, 307, GAMEMENU_ARCHIVE, ASSET_GAMEMENU_RETURN,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod<GuiWindow>(*this, &MainMenuWindow::close));
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &MainMenuWindow::showHelp,
		HELP_GAMEMENU_RETURN));
}

void MainMenuWindow::newGame(int x, int y, int arg) STUB(_parent)

void MainMenuWindow::quitGame(int x, int y, int arg) {
	_parent->close();
	gui_stack->clear();
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
	ConfirmationWindow *w;

	w = new ConfirmationWindow(_parent,
		gameLang->hstrings(HSTR_MAINMENU_ASK_NEWGAME));
	w->setYesCallback(GuiMethod(*this, &MainMenuWindow::newGame));
}

void MainMenuWindow::clickSave(int x, int y, int arg) STUB(_parent)

void MainMenuWindow::clickLoad(int x, int y, int arg) {
	new LoadGameWindow(_parent, 0);
}

void MainMenuWindow::clickQuit(int x, int y, int arg) {
	ConfirmationWindow *w;

	w = new ConfirmationWindow(_parent,
		gameLang->hstrings(HSTR_MAINMENU_ASK_QUIT));
	w->setYesCallback(GuiMethod(*this, &MainMenuWindow::quitGame));
}

void MainMenuWindow::clickSettings(int x, int y, int arg) STUB(_parent)
