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

#ifndef SHIPS_H_
#define SHIPS_H_

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
	int _curSlot, _selShip, _showCombat, _showSupport, _activePlayer;
	int _multiselect;
	Fleet *_fleet;

protected:
	int getSlot(unsigned x, unsigned y) const;
	Ship *getShip(int slot);

	void updateSingleCount(void);
	void toggleSlot(unsigned slot);

public:
	ShipGridWidget(GuiView *parent, unsigned x, unsigned y, unsigned rows,
		unsigned cols, unsigned hspace, unsigned vspace,
		const GameState *game, int activePlayer, Image *slotsel,
		Image *slotframe = NULL);
	~ShipGridWidget(void);

	void setFleet(Fleet *f, int selection);
	void setFilter(int combat, int support);
	void setSelectionMode(int multiselect);
	void setScroll(unsigned pos);
	void selectAll(void);
	void selectNone(void);
	void highlight(int slot);

	unsigned rows(void) const;
	unsigned cols(void) const;
	unsigned selectedShipCount(void) const;
	unsigned selectedCombatCount(void) const;
	unsigned selectedSupportCount(void) const;
	unsigned selectedVisibleCount(void) const;
	unsigned visibleShipCount(void) const;
	char *getSelection(void); // always zeroed when not in multiselect mode
	Ship *selectedShip(void); // always returns NULL in multiselect mode
	Ship *highlightedShip(void);

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
	TextLayout *_shipInfo;
	GuiSprite *_shipOfficer;
	int _scrollgrab, _activePlayer;
	unsigned _infoOffset, _officerETA;

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

	void clearShipInfo(void);
	void generateShipInfo(const Ship *sptr);
	void redrawShipInfo(unsigned curtick);

public:
	FleetListView(GameState *game, int activePlayer);
	~FleetListView(void);

	void redraw(unsigned curtick);

	void open(void);

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

void fleetSummary(StringBuffer &buf, const Fleet &f);

#endif
