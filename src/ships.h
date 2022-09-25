/*
 * This file is part of OpenOrion2
 * Copyright (C) 2022 Martin Doucha
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

#ifndef COLONY_H_
#define COLONY_H_

#include "gui.h"
#include "lbx.h"
#include "galaxy.h"
#include "gamestate.h"

// FIXME: handle captured ships
class ShipAssets {
private:
	ImageAsset _sprites[MAX_PLAYERS + 1][MAX_SHIP_SPRITES];
	const GameState *_game;

public:
	ShipAssets(const GameState *game);

	void load(const uint8_t *palette);

	const Image *getSprite(unsigned builder, unsigned picture) const;
	const Image *getSprite(const Ship *s) const;
};

class ShipGridWidget : public Widget {
private:
	GuiView *_parent;
	Image *_slotsel, *_slotframe;
	ShipAssets _shipimg;
	GuiCallback _onShipHighlight, _onSelectionChange;
	unsigned _rows, _cols, _hspace, _vspace, _scroll;
	unsigned _combatSelCount, _supportSelCount;
	char _selection[MAX_SHIPS];
	int _curSlot, _showCombat, _showSupport, _activePlayer;
	Fleet *_fleet;

protected:
	int getSlot(unsigned x, unsigned y) const;

public:
	ShipGridWidget(GuiView *parent, unsigned x, unsigned y, unsigned rows,
		unsigned cols, unsigned hspace, unsigned vspace,
		const GameState *game, int activePlayer, Image *slotsel,
		Image *slotframe = NULL);
	~ShipGridWidget(void);

	void setFleet(Fleet *f, int selection = 1);
	void setFilter(int combat, int support);
	void setScroll(unsigned pos);
	void selectAll(void);
	void selectNone(void);

	unsigned rows(void) const;
	unsigned cols(void) const;
	unsigned selectedShipCount(void) const;
	unsigned selectedCombatCount(void) const;
	unsigned selectedSupportCount(void) const;
	unsigned selectedVisibleCount(void) const;
	unsigned visibleShipCount(void) const;
	char *getSelection(void);
	Ship *currentShip(void);

	virtual void setShipHighlightCallback(const GuiCallback &callback);
	virtual void setSelectionChangeCallback(const GuiCallback &callback);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class FleetListView : public GuiView {
private:
	ImageAsset _bg;
	GameState *_game;
	GalaxyMinimapWidget *_minimap;
	ShipGridWidget *_grid;
	ScrollBarWidget *_scroll;
	Widget *_allButton, *_scrapButton;
	ToggleWidget *_supportToggle, *_combatToggle;
	LabelWidget *_minimapLabel;
	int _scrollgrab, _activePlayer;

	void initWidgets(void);

protected:
	void starHighlightChanged(int x, int y, int arg);
	void starSelectionChanged(int x, int y, int arg);
	void fleetHighlightChanged(int x, int y, int arg);
	void fleetSelectionChanged(int x, int y, int arg);

	void shipHighlightChanged(int x, int y, int arg);
	void shipSelectionChanged(int x, int y, int arg);

	void filterCombat(int x, int y, int arg);
	void filterSupport(int x, int y, int arg);

	void handleBeginScroll(int x, int y, int arg);
	void handleScroll(int x, int y, int arg);
	void handleEndScroll(int x, int y, int arg);
	void updateScrollbar(void);

public:
	FleetListView(GameState *game, int activePlayer);
	~FleetListView(void);

	void redraw(unsigned curtick);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned buttons);

	void showHelp(int x, int y, int arg);
	void clickPrevFleet(int x, int y, int arg);
	void clickNextFleet(int x, int y, int arg);
	void clickAllButton(int x, int y, int arg);
	void clickRelocate(int x, int y, int arg);
	void clickScrap(int x, int y, int arg);
	void clickLeaders(int x, int y, int arg);
	void clickReturn(int x, int y, int arg);
};

extern const unsigned npcFleetOwnerNames[NPC_FLEET_OWNERS];

#endif
