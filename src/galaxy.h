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

#ifndef GALAXY_H_
#define GALAXY_H_

#include "gui.h"
#include "gamestate.h"

#define GALAXY_ZOOM_LEVELS 4
#define GALAXY_STAR_SIZES 6

class GalaxyMinimapWidget : public Widget {
private:
	ImageAsset _bhole, _freestar, _starimg[MAX_PLAYERS];
	ImageAsset _fleetimg[MAX_FLEET_OWNERS];
	GuiCallback _onStarHighlight, _onStarSelect;
	GuiCallback _onFleetHighlight, _onFleetSelect;
	GameState *_game;
	Fleet *_curFleet, *_selFleet;
	unsigned _startTick;
	int _curStar, _selStar, _activePlayer;

protected:
	unsigned starX(unsigned x);
	unsigned starY(unsigned y);
	unsigned fleetX(const Fleet *f);
	unsigned fleetY(const Fleet *f);
	const Image *getFleetSprite(const Fleet *f);
	const Image *getStarSprite(const Star *s);

	void drawFleet(int x, int y, const Fleet *f,
		unsigned curtick);
	void drawStar(int x, int y, const Star *s, unsigned curtick);

	void findObject(unsigned x, unsigned y, int *rstar, Fleet **rfleet);
	int touchesFleet(unsigned x, unsigned y, const Fleet *f);

public:
	GalaxyMinimapWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, GameState *game, int activePlayer,
		const char *archive, unsigned starAssets, unsigned fleetAssets,
		const uint8_t *palette = NULL);
	~GalaxyMinimapWidget(void);

	Fleet *highlightedFleet(void);
	Fleet *selectedFleet(void);
	int highlightedStar(void);
	int selectedStar(void);

	void highlightFleet(Fleet *f);
	void selectFleet(Fleet *f);
	void highlightStar(int id);
	void selectStar(int id);

	void setStarHighlightCallback(const GuiCallback &callback);
	void setStarSelectCallback(const GuiCallback &callback);
	void setFleetHighlightCallback(const GuiCallback &callback);
	void setFleetSelectCallback(const GuiCallback &callback);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class GalaxyView : public GuiView {
private:
	ImageAsset _bg, _gui, _starimg[STAR_TYPE_COUNT][GALAXY_STAR_SIZES];
	ImageAsset _nebulaimg[NEBULA_TYPE_COUNT][GALAXY_ZOOM_LEVELS];
	ImageAsset _bholeimg[GALAXY_ZOOM_LEVELS];
	ImageAsset _fleetimg[MAX_FLEET_OWNERS][GALAXY_ZOOM_LEVELS];
	ImageAsset _turnDoneLights[MAX_PLAYERS];
	GameState *_game;
	unsigned _zoom, _zoomX, _zoomY, _startTick;
	int _activePlayer;

	void initWidgets(void);

protected:
	// galaxy coordinates to screen coordinates conversion
	int transformX(int x) const;
	int transformY(int y) const;
	int transformFleetX(const Fleet *f) const;
	int transformFleetY(const Fleet *f) const;
	const Image *getFleetSprite(const Fleet *f) const;

	void selectPlayer(void);
	void setPlayer(int player, int a, int b);

	void redrawSidebar(unsigned curtick);

public:
	// Gamestate must be dynamically allocated and GalaxyView takes
	// ownership of the instance
	GalaxyView(GameState *game);
	~GalaxyView(void);

	void open(void);

	void redraw(unsigned curtick);

	void showHelp(int x, int y, int arg);
	void clickGameMenu(int x, int y, int arg);

	void clickColoniesButton(int x, int y, int arg);
	void clickPlanetsButton(int x, int y, int arg);
	void clickFleetsButton(int x, int y, int arg);
	void clickLeadersButton(int x, int y, int arg);
	void clickRacesButton(int x, int y, int arg);
	void clickInfoButton(int x, int y, int arg);

	void clickZoomInButton(int x, int y, int arg);
	void clickZoomOutButton(int x, int y, int arg);
	void clickTurnButton(int x, int y, int arg);
};

class SelectPlayerView : public GuiView {
private:
	const GameState *_game;
	GuiCallback _callback;
	ImageAsset _playerFlags[PLAYER_COUNT];
	ImageAsset _bg, _header, _row, _footer;
	unsigned _humans[PLAYER_COUNT], _playerCount, _animStart, _y;
	int _currentPlayer;

	void initWidgets(void);

public:
	SelectPlayerView(const GameState *game, const GuiCallback &callback);

	void redraw(unsigned curtick);

	void highlightPlayer(int x, int y, int arg);
	void clickPlayer(int x, int y, int arg);
};

class PlanetsListView : public GuiView {
private:
	const GameState *_game;
	int _activePlayer;
	ImageAsset _bg, _planetimg[PLANET_CLIMATE_COUNT][PLANET_SIZE_COUNT];
	unsigned _planetCount, _planets[MAX_PLANETS];

	void initWidgets(void);

protected:
	void updateList(void);

public:
	PlanetsListView(const GameState *game, int activePlayer);
	
	void redraw(unsigned curtick);
	void clickReturn(int x, int y, int arg);
};

class MainMenuWindow : public GuiWindow {
private:
	ImageAsset _bg;
	GameState *_game;

	void initWidgets(const uint8_t *palette);

public:
	// MainMenuWindow does not take ownership of the gamestate
	MainMenuWindow(GuiView *parent, GameState *game);
	~MainMenuWindow(void);

	void redraw(unsigned curtick);

	void showHelp(int x, int y, int arg);
	void clickNew(int x, int y, int arg);
	void clickSave(int x, int y, int arg);
	void clickLoad(int x, int y, int arg);
	void clickQuit(int x, int y, int arg);
	void clickSettings(int x, int y, int arg);
};

#endif
