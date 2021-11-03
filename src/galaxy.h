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

class GalaxyView : public GuiView {
private:
	ImageAsset _bg, _gui, _starimg[STAR_TYPE_COUNT][GALAXY_STAR_SIZES];
	ImageAsset _nebulaimg[NEBULA_TYPE_COUNT][GALAXY_ZOOM_LEVELS];
	ImageAsset _bholeimg[GALAXY_ZOOM_LEVELS];
	ImageAsset _fleetimg[MAX_FLEET_OWNERS][GALAXY_ZOOM_LEVELS];
	GameState *_game;
	unsigned _zoom, _zoomX, _zoomY, _startTick;

	void initWidgets(void);

protected:
	// galaxy coordinates to screen coordinates conversion
	int transformX(int x) const;
	int transformY(int y) const;
	int transformFleetX(const Fleet *f) const;
	int transformFleetY(const Fleet *f) const;
	const Image *getFleetSprite(const Fleet *f) const;

public:
	// Gamestate must be dynamically allocated and GalaxyView takes
	// ownership of the instance
	GalaxyView(GameState *game);
	~GalaxyView(void);

	void open(void);

	void redraw(unsigned curtick);

	void clickGameMenu(int x, int y, int arg);
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

	void clickNew(int x, int y, int arg);
	void clickSave(int x, int y, int arg);
	void clickLoad(int x, int y, int arg);
	void clickQuit(int x, int y, int arg);
	void clickSettings(int x, int y, int arg);
};

#endif
