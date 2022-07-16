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

#include <cstring>

#include "screen.h"
#include "guimisc.h"
#include "lang.h"
#include "ships.h"

#define FLEETLIST_ARCHIVE "fleet.lbx"
#define ASSET_FLEET_GUI 0
#define ASSET_FLEET_SCROLL_UP_BUTTON 2
#define ASSET_FLEET_PREV_FLEET_BUTTON 3
#define ASSET_FLEET_NEXT_FLEET_BUTTON 4
#define ASSET_FLEET_SCROLL_DOWN_BUTTON 5
#define ASSET_FLEET_ALL_BUTTON 6
#define ASSET_FLEET_RELOCATE_BUTTON 7
#define ASSET_FLEET_SCRAP_BUTTON 8
#define ASSET_FLEET_SUPPORT_TOGGLE 9
#define ASSET_FLEET_COMBAT_TOGGLE 10
#define ASSET_FLEET_LEADERS_BUTTON 11
#define ASSET_FLEET_RETURN_BUTTON 12
#define ASSET_FLEET_ALL_BUTTON_DISABLED 13
#define ASSET_FLEET_SCRAP_BUTTON_DISABLED 15
#define ASSET_FLEET_SLOT_SELECTED 17
#define ASSET_FLEET_SLOT_HIGHLIGHTED 18
#define ASSET_FLEET_SHIP_IMAGES 19
#define ASSET_FLEET_STAR_IMAGES 34
#define ASSET_FLEET_NEUTRAL_STAR_IMAGE 42
#define ASSET_FLEET_BHOLE_IMAGE 44
#define ASSET_FLEET_LEADER_IMAGES 45
#define ASSET_FLEET_PALETTE 111

#define SHIPSPRITE_ARCHIVE "ships.lbx"
#define PALSPRITE_ANTARAN 413
#define PALSPRITE_DRAGON 414
#define PALSPRITE_EEL 415
#define PALSPRITE_CRYSTAL 416
#define PALSPRITE_HYDRA 417
#define PALSPRITE_AMOEBA 418
#define PALSPRITE_GUARDIAN 419

#define FLEETLIST_SCROLL_WIDTH 10

static const unsigned monster_palettes[MAX_SHIPTYPES_MONSTER] = {
	PALSPRITE_AMOEBA, PALSPRITE_CRYSTAL, PALSPRITE_DRAGON, PALSPRITE_EEL,
	PALSPRITE_HYDRA
};

static const uint8_t fleetlistScrollTexture[FLEETLIST_SCROLL_WIDTH * 3] = {
	RGB(0x0000dc), RGB(0x0000dc), RGB(0x0000b4), RGB(0x0000b4),
	RGB(0x0c0888), RGB(0x0c0888), RGB(0x00006c), RGB(0x00006c),
	RGB(0x080850), RGB(0x080850)
};

ShipAssets::ShipAssets(const GameState *game) : _game(game) {

}

void ShipAssets::load(const uint8_t *palette) {
	unsigned i, j, pos;
	ImageAsset palimg;
	const uint8_t *pal;

	// Player ships
	for (i = 0, pos = 0; i < MAX_PLAYERS; i++, pos += MAX_SHIP_SPRITES+1) {
		palimg = gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			pos + MAX_SHIP_SPRITES, palette);
		pal = palimg->palette();

		for (j = 0; j < MAX_SHIP_SPRITES; j++) {
			_sprites[i][j] = gameAssets->getImage(
				SHIPSPRITE_ARCHIVE, pos + j, pal);
		}
	}

	// Antaran ships
	palimg = gameAssets->getImage(SHIPSPRITE_ARCHIVE, PALSPRITE_ANTARAN,
		palette);
	pal = palimg->palette();
	j = SHIPSPRITE_ANTARAN;

	for (i = 0; i < MAX_SHIPTYPES_ANTARAN; i++, j++) {
		_sprites[MAX_PLAYERS][j] = gameAssets->getImage(
			SHIPSPRITE_ARCHIVE, pos + j, pal);
	}

	// Orion Guardian
	palimg = gameAssets->getImage(SHIPSPRITE_ARCHIVE, PALSPRITE_GUARDIAN,
		palette);
	_sprites[MAX_PLAYERS][SHIPSPRITE_GUARDIAN] = gameAssets->getImage(
		SHIPSPRITE_ARCHIVE, pos + SHIPSPRITE_GUARDIAN,
		palimg->palette());

	// Monsters
	for (i = 0; i < MAX_SHIPTYPES_MONSTER; i++) {
		palimg = gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			monster_palettes[i], palette);
		pal = palimg->palette();
		_sprites[MAX_PLAYERS][SHIPSPRITE_MONSTER + i] =
			gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			pos + SHIPSPRITE_MONSTER + i, pal);
		_sprites[MAX_PLAYERS][SHIPSPRITE_MINIMONSTER + i] =
			gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			pos + SHIPSPRITE_MINIMONSTER + i, pal);
	}
}

const Image *ShipAssets::getSprite(unsigned builder, unsigned picture) const {
	unsigned shipset;
	const Image *ret;

	if (picture >= MAX_SHIP_SPRITES) {
		throw std::out_of_range("Invalid ship picture ID");
	}

	// Player ships
	if (builder < MAX_PLAYERS) {
		if (builder >= _game->_playerCount) {
			throw std::out_of_range("Invalid player ID");
		}

		shipset = _game->_players[builder].color;
		return (const Image*)_sprites[shipset][picture];
	}

	// Antarans and monsters
	ret = (const Image*)_sprites[MAX_PLAYERS][picture];

	if (!ret) {
		throw std::invalid_argument("Invalid monster picture ID");
	}

	return ret;
}

const Image *ShipAssets::getSprite(const Ship *s) const {
	return getSprite(s->design.builder, s->design.picture);
}

ShipGridWidget::ShipGridWidget(GuiView *parent, unsigned x, unsigned y,
	unsigned rows, unsigned cols, unsigned hspace, unsigned vspace,
	const GameState *game, int activePlayer, Image *slotsel,
	Image *slotframe) :
	Widget(x, y, cols * slotsel->width() + (cols - 1) * hspace,
	rows * slotsel->height() + (rows - 1) * vspace), _parent(parent),
	_slotsel(slotsel), _slotframe(slotframe), _shipimg(game), _rows(rows),
	_cols(cols), _hspace(hspace), _vspace(vspace), _scroll(0),
	_combatSelCount(0), _supportSelCount(0), _curSlot(-1), _showCombat(1),
	_showSupport(1), _activePlayer(activePlayer), _fleet(NULL) {

	if (!rows || !cols) {
		throw std::invalid_argument(
			"Ship grid must have at least 1 row and column");
	}

	_shipimg.load(_slotsel->palette());
	gameAssets->takeAsset(_slotsel);

	if (_slotframe) {
		try {
			gameAssets->takeAsset(_slotframe);
		} catch (...) {
			gameAssets->freeAsset(_slotsel);
			throw;
		}
	}

	memset(_selection, 0, MAX_SHIPS * sizeof(char));
}

ShipGridWidget::~ShipGridWidget(void) {
	gameAssets->freeAsset(_slotsel);

	if (_slotframe) {
		gameAssets->freeAsset(_slotframe);
	}
}

int ShipGridWidget::getSlot(unsigned x, unsigned y) const {
	unsigned sw, sh;

	sw = _slotsel->width();
	sh = _slotsel->height();
	x -= getX();
	y -= getY();

	// Area between grid cells
	if (x % (sw + _hspace) >= sw || y % (sh + _vspace) >= sh) {
		return -1;
	}

	return _cols * (_scroll + y / (sh + _vspace)) + x / (sw + _hspace);
}

void ShipGridWidget::setFleet(Fleet *f, int selection) {
	_fleet = f;
	_curSlot = -1;

	if (f) {
		memset(_selection, selection, f->shipCount() * sizeof(char));
	}

	_combatSelCount = selection && f ? f->combatCount() : 0;
	_supportSelCount = selection && f ? f->supportCount() : 0;
}

void ShipGridWidget::setFilter(int combat, int support) {
	_curSlot = -1;
	_showCombat = combat;
	_showSupport = support;
}

void ShipGridWidget::setScroll(unsigned pos) {
	_scroll = pos;
}

void ShipGridWidget::selectAll(void) {
	if (!_fleet) {
		return;
	}

	memset(_selection, 1, _fleet->shipCount() * sizeof(char));
	_combatSelCount = _fleet->combatCount();
	_supportSelCount = _fleet->supportCount();
}

void ShipGridWidget::selectNone(void) {
	if (_fleet) {
		memset(_selection, 0, _fleet->shipCount() * sizeof(char));
	}

	_combatSelCount = 0;
	_supportSelCount = 0;
}

unsigned ShipGridWidget::rows(void) const {
	return _rows;
}

unsigned ShipGridWidget::cols(void) const {
	return _cols;
}

unsigned ShipGridWidget::selectedShipCount(void) const {
	return _combatSelCount + _supportSelCount;
}

unsigned ShipGridWidget::selectedCombatCount(void) const {
	return _combatSelCount;
}

unsigned ShipGridWidget::selectedSupportCount(void) const {
	return _supportSelCount;
}

unsigned ShipGridWidget::selectedVisibleCount(void) const {
	unsigned ret = _showCombat ? _combatSelCount : 0;

	ret += _showSupport ? _supportSelCount : 0;
	return ret;
}

unsigned ShipGridWidget::visibleShipCount(void) const {
	unsigned ret = 0;

	if (!_fleet) {
		return 0;
	}

	ret = _showCombat ? _fleet->combatCount() : 0;
	ret += _showSupport ? _fleet->supportCount() : 0;
	return ret;
}

char *ShipGridWidget::getSelection(void) {
	return _selection;
}

Ship *ShipGridWidget::currentShip(void) {
	unsigned offset = 0;

	if (_curSlot < 0 || !_fleet) {
		return NULL;
	}

	if (!_showCombat) {
		offset = _fleet->combatCount();
	}

	return _fleet->getShip(offset + _curSlot);
}

void ShipGridWidget::setShipHighlightCallback(const GuiCallback &callback) {
	_onShipHighlight = callback;
}

void ShipGridWidget::setSelectionChangeCallback(const GuiCallback &callback) {
	_onSelectionChange = callback;
}

void ShipGridWidget::handleMouseMove(int x, int y, unsigned buttons) {
	int slot, count;

	slot = getSlot(x, y);
	count = visibleShipCount();

	if (slot >= 0 && slot < count && slot != _curSlot &&
		_fleet->getOwner() == _activePlayer) {
		_curSlot = slot;
		_onShipHighlight(x, y);
		return;
	}

	Widget::handleMouseMove(x, y, buttons);
}

void ShipGridWidget::handleMouseUp(int x, int y, unsigned button) {
	unsigned offset, combatCount;
	int slot, count, change;

	slot = getSlot(x, y);
	count = visibleShipCount();

	if (slot < 0 || slot >= count || _fleet->getOwner() != _activePlayer) {
		Widget::handleMouseUp(x, y, button);
		return;
	}

	combatCount = _fleet->combatCount();
	offset = _showCombat ? 0 : combatCount;

	switch (button) {
	case MBUTTON_LEFT:
		change = _selection[offset + slot] ? -1 : 1;
		_selection[offset + slot] = !_selection[offset + slot];

		if (offset + (unsigned)slot < combatCount) {
			_combatSelCount += change;
		} else {
			_supportSelCount += change;
		}

		_onSelectionChange(x, y);
		return;

	case MBUTTON_RIGHT:
		new MessageBoxWindow(_parent,
			"Ship info window not implemented");
		return;
	}
}

void ShipGridWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, offset, count, sw, sh, xpos, ypos;
	int dx, dy;
	const Image *img;

	if (!_fleet || isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	count = visibleShipCount();
	offset = _showCombat ? 0 : _fleet->combatCount();
	offset += _scroll * _cols;
	sw = _slotsel->width() + _hspace;
	sh = _slotsel->height() + _vspace;

	for (i = 0; i < _rows * _cols && i + _scroll * _cols < count; i++) {
		xpos = sw * (i % _cols);
		ypos = sh * (i / _cols);

		if (_selection[offset + i]) {
			_slotsel->draw(x + xpos, y + ypos);
		}

		img = _shipimg.getSprite(_fleet->getShip(offset + i));
		dx = ((int)_slotsel->width() - (int)img->width()) / 2;
		dy = ((int)_slotsel->height() - (int)img->height()) / 2;
		img->draw(x + xpos + dx, y + ypos + dy);

		if (_slotframe && _curSlot == int(i + _scroll * _cols)) {
			_slotframe->draw(x + xpos - 1, y + ypos - 1);
		}
	}
}

FleetListView::FleetListView(GameState *game, int activePlayer) :
	_game(game), _minimap(NULL), _grid(NULL), _scroll(NULL),
	_allButton(NULL), _scrapButton(NULL), _supportToggle(NULL),
	_combatToggle(NULL), _scrollgrab(0), _activePlayer(activePlayer) {

	ImageAsset palimg;
	const uint8_t *pal;

	palimg = gameAssets->getImage(FLEETLIST_ARCHIVE, ASSET_FLEET_PALETTE);
	pal = palimg->palette();
	_bg = gameAssets->getImage(FLEETLIST_ARCHIVE, ASSET_FLEET_GUI, pal);

	initWidgets();
}

FleetListView::~FleetListView(void) {

}

void FleetListView::initWidgets(void) {
	ImageAsset slotsel, slotframe;
	const uint8_t *pal = _bg->palette();
	Widget *w;

	_minimap = new GalaxyMinimapWidget(15, 52, 305, 182, _game,
		_activePlayer, FLEETLIST_ARCHIVE, ASSET_FLEET_STAR_IMAGES,
		ASSET_FLEET_SHIP_IMAGES, pal);
	addWidget(_minimap);
	_minimap->setStarSelectCallback(GuiMethod(*this,
		&FleetListView::starSelectionChanged));
	_minimap->setFleetSelectCallback(GuiMethod(*this,
		&FleetListView::fleetSelectionChanged));
	_minimap->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_MINIMAP));

	slotsel = gameAssets->getImage(FLEETLIST_ARCHIVE,
		ASSET_FLEET_SLOT_SELECTED, pal);
	slotframe = gameAssets->getImage(FLEETLIST_ARCHIVE,
		ASSET_FLEET_SLOT_HIGHLIGHTED, pal);
	_grid = new ShipGridWidget(this, 345, 54, 5, 4, 3, 2, _game,
		_activePlayer, (Image*)slotsel, (Image*)slotframe);
	addWidget(_grid);
	_grid->setShipHighlightCallback(GuiMethod(*this,
		&FleetListView::shipHighlightChanged));
	_grid->setSelectionChangeCallback(GuiMethod(*this,
		&FleetListView::shipSelectionChanged));
	_grid->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SHIP_GRID));

	_scroll = new ScrollBarWidget(607, 89, FLEETLIST_SCROLL_WIDTH, 229,
		5, 5, fleetlistScrollTexture);
	addWidget(_scroll);
	_scroll->setBeginScrollCallback(GuiMethod(*this,
		&FleetListView::handleBeginScroll));
	_scroll->setScrollCallback(GuiMethod(*this,
		&FleetListView::handleScroll));
	_scroll->setEndScrollCallback(GuiMethod(*this,
		&FleetListView::handleEndScroll));
	_scroll->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SCROLLBAR));

	w = createWidget(606, 59, 13, 24);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollMinus));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_SCROLL_UP_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SCROLLBAR));

	w = createWidget(605, 325, 14, 25);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollPlus));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_SCROLL_DOWN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SCROLLBAR));

	w = createWidget(19, 249, 32, 19);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickPrevFleet));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_PREV_FLEET_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_PREV_BUTTON));

	w = createWidget(283, 249, 31, 19);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickNextFleet));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_NEXT_FLEET_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_NEXT_BUTTON));

	_allButton = createWidget(348, 380, 74, 28);
	_allButton->setDisabledSprite(FLEETLIST_ARCHIVE,
		ASSET_FLEET_ALL_BUTTON_DISABLED, pal, 0);
	_allButton->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickAllButton));
	_allButton->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_ALL_BUTTON, pal, 1);
	_allButton->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_ALL_BUTTON));
	_allButton->disable(1);

	w = createWidget(441, 380, 89, 28);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickRelocate));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_RELOCATE_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_RELOCATE_BUTTON));

	_scrapButton = createWidget(549, 380, 73, 28);
	_scrapButton->setDisabledSprite(FLEETLIST_ARCHIVE,
		ASSET_FLEET_SCRAP_BUTTON_DISABLED, pal, 0);
	_scrapButton->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickScrap));
	_scrapButton->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_SCRAP_BUTTON, pal, 1);
	_scrapButton->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SCRAP_BUTTON));
	_scrapButton->disable(1);

	w = createWidget(342, 430, 73, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickLeaders));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_LEADERS_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_LEADERS_BUTTON));

	_supportToggle = new ToggleWidget(425, 435, 61, 19, FLEETLIST_ARCHIVE,
		ASSET_FLEET_SUPPORT_TOGGLE, pal, 1);
	addWidget(_supportToggle);
	_supportToggle->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::filterSupport));
	_supportToggle->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SUPPORT_TOGGLE));

	_combatToggle = new ToggleWidget(487, 435, 60, 19, FLEETLIST_ARCHIVE,
		ASSET_FLEET_COMBAT_TOGGLE, pal, 1);
	addWidget(_combatToggle);
	_combatToggle->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::filterCombat));
	_combatToggle->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_COMBAT_TOGGLE));

	w = createWidget(556, 430, 73, 27);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::clickReturn));
	w->setClickSprite(MBUTTON_LEFT, FLEETLIST_ARCHIVE,
		ASSET_FLEET_RETURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_RETURN_BUTTON));

	w = createWidget(66, 247, 202, 23);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_NAME_BAR));

	w = createWidget(15, 280, 305, 185);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SHIP_INFO));
}

void FleetListView::starSelectionChanged(int x, int y, int arg) {
	// TODO: implement movement and relocation commands
	_minimap->selectStar(-1);
}

void FleetListView::fleetSelectionChanged(int x, int y, int arg) {
	int select = 0;
	Fleet *f = _minimap->selectedFleet();

	if (f) {
		select = f->getOwner() == _activePlayer;
	}

	_grid->setFleet(f, select);

	if (!_grid->visibleShipCount()) {
		if (f->combatCount()) {
			_combatToggle->setValue(1);
		} else {
			_supportToggle->setValue(1);
		}

		_grid->setFilter(_combatToggle->value(),
			_supportToggle->value());
	}

	_allButton->disable(!select || !_grid->visibleShipCount());
	updateScrollbar();
	shipHighlightChanged(0, 0, 0);
	shipSelectionChanged(0, 0, 0);
}

void FleetListView::shipHighlightChanged(int x, int y, int arg) {
	// TODO: show ship info
}

void FleetListView::shipSelectionChanged(int x, int y, int arg) {
	_scrapButton->disable(!_grid->selectedVisibleCount());
}

void FleetListView::filterCombat(int x, int y, int arg) {
	unsigned combat, support;
	Fleet *f = _minimap->selectedFleet();

	combat = _combatToggle->value();
	support = _supportToggle->value();

	if (f) {
		combat = combat || !f->supportCount();
		_combatToggle->setValue(combat);
	}

	support = support || !combat;
	_supportToggle->setValue(support);
	_grid->setFilter(combat, support);
	_allButton->disable(f->getOwner() != _activePlayer ||
		!_grid->visibleShipCount());
	updateScrollbar();
	shipHighlightChanged(0, 0, 0);
	shipSelectionChanged(0, 0, 0);
}

void FleetListView::filterSupport(int x, int y, int arg) {
	unsigned combat, support;
	Fleet *f = _minimap->selectedFleet();

	combat = _combatToggle->value();
	support = _supportToggle->value();

	if (f) {
		support = support || !f->combatCount();
		_supportToggle->setValue(support);
	}

	combat = combat || !support;
	_combatToggle->setValue(combat);
	_grid->setFilter(combat, support);
	_allButton->disable(f->getOwner() != _activePlayer ||
		!_grid->visibleShipCount());
	updateScrollbar();
	shipHighlightChanged(0, 0, 0);
	shipSelectionChanged(0, 0, 0);
}

void FleetListView::handleBeginScroll(int x, int y, int arg) {
	_scrollgrab = 1;
}

void FleetListView::handleScroll(int x, int y, int arg) {
	_grid->setScroll(_scroll->position());
}

void FleetListView::handleEndScroll(int x, int y, int arg) {
	_scrollgrab = 0;
	handleMouseMove(x, y, 0);	// force active widget lookup
}

void FleetListView::updateScrollbar(void) {
	unsigned cols = _grid->cols();

	_scroll->setRange((_grid->visibleShipCount() + cols - 1) / cols);
}

void FleetListView::redraw(unsigned curtick) {
	clearScreen();
	_bg->draw(0, 0);

	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void FleetListView::handleMouseMove(int x, int y, unsigned buttons) {
	if (_scrollgrab) {
		_scroll->handleMouseMove(x, y, buttons);
		return;
	}

	GuiView::handleMouseMove(x, y, buttons);
}

void FleetListView::handleMouseUp(int x, int y, unsigned button) {
	if (_scrollgrab) {
		_scroll->handleMouseUp(x, y, button);
		return;
	}

	GuiView::handleMouseUp(x, y, button);
}

void FleetListView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _bg->palette());
}

void FleetListView::clickPrevFleet(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented");
}

void FleetListView::clickNextFleet(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented");
}

void FleetListView::clickAllButton(int x, int y, int arg) {
	Fleet *f = _minimap->selectedFleet();

	if (!f || f->getOwner() != _activePlayer) {
		return;
	}

	if (_grid->selectedVisibleCount() == _grid->visibleShipCount()) {
		_grid->selectNone();
	} else {
		_grid->selectAll();
	}

	shipSelectionChanged(x, y, 0);
}

void FleetListView::clickRelocate(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented");
}

void FleetListView::clickScrap(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented");
}

void FleetListView::clickLeaders(int x, int y, int arg) {
	new MessageBoxWindow(this, "Not implemented");
}

void FleetListView::clickReturn(int x, int y, int arg) {
	exitView();
}
