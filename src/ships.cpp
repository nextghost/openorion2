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
#include "galaxy.h"
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
#define FLEETLIST_SHIPINFO_WIDTH 299
#define FLEETLIST_SHIPINFO_HEIGHT 175
#define FLEETLIST_SHIPINFO_COL2 155

static const unsigned monster_palettes[MAX_SHIPTYPES_MONSTER] = {
	PALSPRITE_AMOEBA, PALSPRITE_CRYSTAL, PALSPRITE_DRAGON, PALSPRITE_EEL,
	PALSPRITE_HYDRA
};

static const uint8_t fleetlistScrollTexture[FLEETLIST_SCROLL_WIDTH * 3] = {
	RGB(0x0000dc), RGB(0x0000dc), RGB(0x0000b4), RGB(0x0000b4),
	RGB(0x0c0888), RGB(0x0c0888), RGB(0x00006c), RGB(0x00006c),
	RGB(0x080850), RGB(0x080850)
};

static const uint8_t fleetlistOfficerBlueFrame[] = {
	RGB(0x5c7ca0), RGB(0x506c90), RGB(0x384c6c), RGB(0x445c80)
};

static const uint8_t fleetlistOfficerRedFrame[] = {
	RGB(0x9c302c), RGB(0x882020), RGB(0x641010), RGB(0x741818)
};

static const unsigned shipTypeHelpEntry[MAX_SHIP_TYPES] = {
	0, HELP_TECH_COLONY_SHIP, HELP_TECH_TRANSPORT, 0,
	HELP_TECH_OUTPOST_SHIP
};

ShipAssets::ShipAssets(const GameState *game) : _game(game) {

}

void ShipAssets::load(const uint8_t *palette) {
	unsigned i, j, pos;
	ImageAsset palimg[MAX_PLAYERS + 1];
	const uint8_t *pals[MAX_PLAYERS + 1];

	for (i = 0; i < MAX_PLAYERS; i++) {
		palimg[i] = gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			(i + 1) * MAX_SHIP_SPRITES + i, palette);
		pals[i] = palimg[i]->palette();
	}

	palimg[MAX_PLAYERS] = gameAssets->getImage(SHIPSPRITE_ARCHIVE,
		PALSPRITE_ANTARAN, palette);
	pals[MAX_PLAYERS] = palimg[MAX_PLAYERS]->palette();

	// Player ships
	for (i = 0, pos = 0; i < MAX_PLAYERS; i++, pos += MAX_SHIP_SPRITES+1) {
		for (j = 0; j < MAX_SHIP_SPRITES; j++) {
			_sprites[i][j] = gameAssets->getImage(
				SHIPSPRITE_ARCHIVE, pos + j, pals,
				MAX_PLAYERS);
		}
	}

	// Antaran ships
	j = SHIPSPRITE_ANTARAN;

	for (i = 0; i < MAX_SHIPTYPES_ANTARAN; i++, j++) {
		_sprites[MAX_PLAYERS][j] = gameAssets->getImage(
			SHIPSPRITE_ARCHIVE, pos + j, pals, MAX_PLAYERS + 1);
	}

	// Orion Guardian
	palimg[0] = gameAssets->getImage(SHIPSPRITE_ARCHIVE, PALSPRITE_GUARDIAN,
		palette);
	_sprites[MAX_PLAYERS][SHIPSPRITE_GUARDIAN] = gameAssets->getImage(
		SHIPSPRITE_ARCHIVE, pos + SHIPSPRITE_GUARDIAN,
		palimg[0]->palette());

	// Monsters
	for (i = 0; i < MAX_SHIPTYPES_MONSTER; i++) {
		palimg[0] = gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			monster_palettes[i], palette);
		pals[0] = palimg[0]->palette();
		_sprites[MAX_PLAYERS][SHIPSPRITE_MONSTER + i] =
			gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			pos + SHIPSPRITE_MONSTER + i, pals[0]);
		_sprites[MAX_PLAYERS][SHIPSPRITE_MINIMONSTER + i] =
			gameAssets->getImage(SHIPSPRITE_ARCHIVE,
			pos + SHIPSPRITE_MINIMONSTER + i, pals[0]);
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
	_combatSelCount(0), _supportSelCount(0), _curSlot(-1), _selShip(-1),
	_showCombat(1), _showSupport(1), _activePlayer(activePlayer),
	_multiselect(1), _fleet(NULL) {

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

void ShipGridWidget::updateSingleCount(void) {
	unsigned combatCount;

	if (_selShip < 0) {
		_combatSelCount = _supportSelCount = 0;
	} else {
		combatCount = _fleet->combatCount();
		_combatSelCount = unsigned(_selShip) < combatCount ? 1 : 0;
		_supportSelCount = unsigned(_selShip) < combatCount ? 0 : 1;
	}
}

void ShipGridWidget::toggleSlot(unsigned slot) {
	unsigned combatCount;
	int change;

	if (slot >= visibleShipCount()) {
		return;
	}

	combatCount = _fleet->combatCount();
	slot += _showCombat ? 0 : combatCount;

	if (!_multiselect) {
		_selShip = (_selShip == (int)slot) ? -1 : slot;
		updateSingleCount();
		return;
	}

	change = _selection[slot] ? -1 : 1;
	_selection[slot] = !_selection[slot];

	if (slot < combatCount) {
		_combatSelCount += change;
	} else {
		_supportSelCount += change;
	}
}

void ShipGridWidget::setFleet(Fleet *f, int selection) {
	_curSlot = -1;
	_selShip = -1;
	_fleet = f;

	if (!f) {
		_combatSelCount = _supportSelCount = 0;
		return;
	}

	if (f->getOwner() != _activePlayer) {
		selection = _multiselect ? 0 : -1;
	}

	if (!_multiselect) {
		selectNone();
		_selShip = selection < (int)f->shipCount() ? selection : -1;
		updateSingleCount();
		return;
	}

	memset(_selection, selection, f->shipCount() * sizeof(char));
	_combatSelCount = selection && f ? f->combatCount() : 0;
	_supportSelCount = selection && f ? f->supportCount() : 0;
}

void ShipGridWidget::setFilter(int combat, int support) {
	_curSlot = -1;
	_showCombat = combat;
	_showSupport = support;
}

void ShipGridWidget::setSelectionMode(int multiselect) {
	unsigned i;
	int oldsel = _selShip;

	if ((_multiselect && multiselect) || (!_multiselect && !multiselect)) {
		return;
	}

	_multiselect = multiselect;

	if (_multiselect && oldsel >= 0) {
		selectNone();
		toggleSlot(oldsel);
	} else if (!_multiselect && selectedShipCount() == 1) {
		for (i = 0; i < _fleet->shipCount() && !_selection[i]; i++);
		_selShip = i < _fleet->shipCount() ? i : -1;
		updateSingleCount();
	} else {
		selectNone();
	}
}

void ShipGridWidget::setScroll(unsigned pos) {
	_scroll = pos;
}

void ShipGridWidget::selectAll(void) {
	if (!_fleet || !_multiselect) {
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

	_selShip = -1;
	_combatSelCount = 0;
	_supportSelCount = 0;
}

void ShipGridWidget::highlight(int slot) {
	int count;

	count = visibleShipCount();

	if (slot < 0 || (slot < count && _fleet->getOwner() == _activePlayer)) {
		_curSlot = slot;
	}
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

Ship *ShipGridWidget::selectedShip(void) {
	return (_fleet && _selShip >= 0) ? _fleet->getShip(_selShip) : NULL;
}

Ship *ShipGridWidget::highlightedShip(void) {
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
	int slot, count;

	slot = getSlot(x, y);
	count = visibleShipCount();

	if (slot < 0 || slot >= count || _fleet->getOwner() != _activePlayer) {
		Widget::handleMouseUp(x, y, button);
		return;
	}

	switch (button) {
	case MBUTTON_LEFT:
		toggleSlot(slot);
		_onSelectionChange(x, y);
		return;

	case MBUTTON_RIGHT:
		new MessageBoxWindow(_parent,
			"Ship info window not implemented");
		return;
	}
}

void ShipGridWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, offset, scrolloff, count, sw, sh, xpos, ypos, color;
	int dx, dy, selected;
	const Image *img;
	const Fleet *f = _fleet;

	if (!f || isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	count = visibleShipCount();
	scrolloff = _scroll * _cols;
	offset = _showCombat ? 0 : f->combatCount();
	offset += scrolloff;
	sw = _slotsel->width() + _hspace;
	sh = _slotsel->height() + _vspace;

	color = f->getColor();
	// Non-Antaran monsters have only 1 color variant
	color = color <= MAX_PLAYERS ? color : 0;

	for (i = 0; i < _rows * _cols && i + scrolloff < count; i++) {
		xpos = sw * (i % _cols);
		ypos = sh * (i / _cols);

		if (_multiselect) {
			selected = _selection[offset + i];
		} else {
			selected = _selShip == int(offset + i);
		}

		if (selected) {
			_slotsel->draw(x + xpos, y + ypos);
		}

		img = _shipimg.getSprite(f->getShip(offset + i));
		dx = ((int)_slotsel->width() - (int)img->width()) / 2;
		dy = ((int)_slotsel->height() - (int)img->height()) / 2;
		img->draw(x + xpos + dx, y + ypos + dy,
			color * img->frameCount());

		if (_slotframe && _curSlot == int(i + scrolloff)) {
			_slotframe->draw(x + xpos - 1, y + ypos - 1);
		}
	}
}

FleetListView::FleetListView(GameState *game, int activePlayer) :
	_game(game), _minimap(NULL), _grid(NULL), _scroll(NULL),
	_allButton(NULL), _scrapButton(NULL), _supportToggle(NULL),
	_combatToggle(NULL), _minimapLabel(NULL), _shipInfo(NULL),
	_shipOfficer(NULL), _scrollgrab(0), _activePlayer(activePlayer),
	_infoOffset(0), _officerETA(0) {

	ImageAsset palimg;
	const uint8_t *pal;

	// Fleet list view uses 80 colors from galaxy view palette
	palimg = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	palimg = gameAssets->getImage(FLEETLIST_ARCHIVE, ASSET_FLEET_PALETTE,
		palimg->palette());
	pal = palimg->palette();
	_bg = gameAssets->getImage(FLEETLIST_ARCHIVE, ASSET_FLEET_GUI, pal);

	initWidgets();
}

FleetListView::~FleetListView(void) {
	delete _shipInfo;
	delete _shipOfficer;
}

void FleetListView::initWidgets(void) {
	ImageAsset slotsel, slotframe;
	const uint8_t *pal = _bg->palette();
	Widget *w;

	_minimap = new GalaxyMinimapWidget(15, 52, 305, 182, _game,
		_activePlayer, FLEETLIST_ARCHIVE, ASSET_FLEET_STAR_IMAGES,
		ASSET_FLEET_SHIP_IMAGES, pal);
	addWidget(_minimap);
	_minimap->setStarHighlightCallback(GuiMethod(*this,
		&FleetListView::starHighlightChanged));
	_minimap->setStarSelectCallback(GuiMethod(*this,
		&FleetListView::starSelectionChanged));
	_minimap->setFleetHighlightCallback(GuiMethod(*this,
		&FleetListView::fleetHighlightChanged));
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
		ASSET_FLEET_SUPPORT_TOGGLE, pal, 0, 1, 1);
	addWidget(_supportToggle);
	_supportToggle->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*this, &FleetListView::filterSupport));
	_supportToggle->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SUPPORT_TOGGLE));

	_combatToggle = new ToggleWidget(487, 435, 60, 19, FLEETLIST_ARCHIVE,
		ASSET_FLEET_COMBAT_TOGGLE, pal, 0, 1, 1);
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

	_minimapLabel = new LabelWidget(66, 247, 202, 23);
	addWidget(_minimapLabel);
	_minimapLabel->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_NAME_BAR));

	w = createWidget(15, 280, 305, 185);
	w->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &FleetListView::showHelp,
		HELP_FLEET_SHIP_INFO));
}

void FleetListView::starHighlightChanged(int x, int y, int arg) {
	int star;
	unsigned color = FONT_COLOR_FLEETLIST_STAR_NEUTRAL;
	const Star *sptr;

	star = _minimap->highlightedStar();

	if (star < 0) {
		_minimapLabel->clear();
		return;
	}

	sptr = _game->_starSystems + star;

	if (sptr->spectralClass >= SpectralClass::BlackHole) {
		_minimapLabel->clear();
		return;
	}

	if (!_game->isStarExplored(sptr, _activePlayer)) {
		const char *str = gameLang->hstrings(HSTR_STAR_UNEXPLORED);

		_minimapLabel->setText(str, FONTSIZE_MEDIUM, color,
			OUTLINE_FULL, ALIGN_CENTER);
		return;
	}

	if (sptr->owner >= 0) {
		color = FONT_COLOR_FLEETLIST_STAR_RED;
		color += _game->_players[sptr->owner].color;
	}

	if (sptr->officerIndex[_activePlayer] >= 0) {
		unsigned idx = sptr->officerIndex[_activePlayer];
		StringBuffer buf;

		buf.printf("%s (%s)", sptr->name, _game->_leaders[idx].name);
		_minimapLabel->setText(buf.c_str(), FONTSIZE_MEDIUM, color,
			OUTLINE_FULL, ALIGN_CENTER);
	} else {
		_minimapLabel->setText(sptr->name, FONTSIZE_MEDIUM, color,
			OUTLINE_FULL, ALIGN_CENTER);
	}
}

void FleetListView::starSelectionChanged(int x, int y, int arg) {
	// TODO: implement movement and relocation commands
	_minimap->selectStar(-1);
}

void FleetListView::fleetHighlightChanged(int x, int y, int arg) {
	unsigned owner, color;
	const Fleet *f = _minimap->highlightedFleet();
	StringBuffer buf;

	if (!f) {
		_minimapLabel->clear();
		return;
	}

	owner = f->getOwner();
	color = FONT_COLOR_FLEETLIST_FLEET_RED;
	fleetSummary(buf, *f);

	if (owner > MAX_PLAYERS) {
		_minimapLabel->setText(buf.c_str(), FONTSIZE_BIG,
			FONT_COLOR_FLEETLIST_FLEET_MONSTER, OUTLINE_FULL,
			ALIGN_CENTER);
		return;
	}

	if (owner < MAX_PLAYERS) {
		color += f->getColor();
	}

	_minimapLabel->setText(buf.c_str(), FONTSIZE_SMALLER, color,
		OUTLINE_FULL, ALIGN_CENTER);
}

void FleetListView::fleetSelectionChanged(int x, int y, int arg) {
	int select = 0;
	Fleet *f = _minimap->selectedFleet();

	if (f) {
		select = f->getOwner() == _activePlayer;
	}

	_grid->setFleet(f, select);

	if (f && !_grid->visibleShipCount()) {
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
	_minimapLabel->clear();
	generateShipInfo(_grid->highlightedShip());
}

void FleetListView::shipSelectionChanged(int x, int y, int arg) {
	_scrapButton->disable(!_grid->selectedVisibleCount());
}

void FleetListView::filterCombat(int x, int y, int arg) {
	unsigned combat, support, selectable = 0;
	Fleet *f = _minimap->selectedFleet();

	combat = _combatToggle->value();
	support = _supportToggle->value();

	if (f) {
		combat = combat || !f->supportCount();
		_combatToggle->setValue(combat);
		selectable = f->getOwner() == _activePlayer;
	}

	support = support || !combat;
	_supportToggle->setValue(support);
	_grid->setFilter(combat, support);
	_allButton->disable(!selectable || !_grid->visibleShipCount());
	updateScrollbar();
	shipHighlightChanged(0, 0, 0);
	shipSelectionChanged(0, 0, 0);
}

void FleetListView::filterSupport(int x, int y, int arg) {
	unsigned combat, support, selectable = 0;
	Fleet *f = _minimap->selectedFleet();

	combat = _combatToggle->value();
	support = _supportToggle->value();

	if (f) {
		support = support || !f->combatCount();
		_supportToggle->setValue(support);
		selectable = f->getOwner() == _activePlayer;
	}

	combat = combat || !support;
	_combatToggle->setValue(combat);
	_grid->setFilter(combat, support);
	_allButton->disable(!selectable || !_grid->visibleShipCount());
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
	selectCurrentWidget(x, y, 0);
}

void FleetListView::updateScrollbar(void) {
	unsigned cols = _grid->cols();

	_scroll->setRange((_grid->visibleShipCount() + cols - 1) / cols);
}

void FleetListView::clearShipInfo(void) {
	TextLayout *info = _shipInfo;
	GuiSprite *sprite = _shipOfficer;

	_shipInfo = NULL;
	_shipOfficer = NULL;
	_officerETA = 0;

	if (info) {
		info->discard();
	}

	if (sprite) {
		sprite->discard();
	}
}

void FleetListView::generateShipInfo(const Ship *sptr) {
	unsigned i, x1, x2, y, idx, count, lineheight, eta = 0;
	TextLayout *newInfo;
	GuiSprite *sprite = NULL;
	Font *fnt;
	const ShipWeapon *wpn;
	const char *str;

	if (!sptr) {
		clearShipInfo();
		return;
	}

	newInfo = new TextLayout;

	if (sptr->design.type != ShipType::COMBAT_SHIP) {
		const struct HelpText *help;

		help = gameLang->help(shipTypeHelpEntry[sptr->design.type]);
		newInfo->setFont(FONTSIZE_MEDIUM, FONT_COLOR_HELP, 2,
			OUTLINE_NONE, 2);
		newInfo->appendText(help->text, 0, 0, FLEETLIST_SHIPINFO_WIDTH);

		clearShipInfo();
		_infoOffset = (FLEETLIST_SHIPINFO_HEIGHT-newInfo->height())/2;
		_shipInfo = newInfo;
		return;
	}

	StringBuffer buf;

	fnt = gameFonts->getFont(FONTSIZE_SMALLER);
	lineheight = fnt->height() + 1;

	newInfo->setFont(FONTSIZE_MEDIUM, FONT_COLOR_HELP, 1, OUTLINE_NONE, 2);
	newInfo->appendText(sptr->design.name, 0, 0, FLEETLIST_SHIPINFO_WIDTH);
	str = gameLang->misctext(TXT_MISC_KENTEXT,
		KEN_SHIPCREW_GREEN + sptr->crewLevel);
	buf.printf(str, (int)sptr->crewExp);
	newInfo->setFont(FONTSIZE_SMALL, FONT_COLOR_HELP, 2);
	newInfo->appendText(buf.c_str(), 0, newInfo->height(),
		FLEETLIST_SHIPINFO_WIDTH);
	str = gameLang->techname(TNAME_SHIELDTYPE_NONE + sptr->design.shield);
	newInfo->appendText(str, 0, newInfo->height(),
		FLEETLIST_SHIPINFO_WIDTH);
	x1 = 0;
	x2 = 115;
	y = newInfo->height();
	newInfo->appendText(gameLang->hstrings(HSTR_SHIPINFO_BEAM_OCV), 0, y,
		FLEETLIST_SHIPINFO_WIDTH);
	buf.printf("%+d", _game->shipBeamOffense(sptr, 1));
	newInfo->appendText(buf.c_str(), 0, y, x2, ALIGN_RIGHT);

	if (sptr->officer >= 0) {
		y = newInfo->height();
	} else {
		x1 = FLEETLIST_SHIPINFO_COL2;
		x2 = FLEETLIST_SHIPINFO_WIDTH - 10;
	}

	newInfo->appendText(gameLang->hstrings(HSTR_SHIPINFO_BEAM_DCV), x1, y,
		FLEETLIST_SHIPINFO_WIDTH);
	buf.printf("%+d", _game->shipBeamDefense(sptr, 1));
	newInfo->appendText(buf.c_str(), x1, y, x2, ALIGN_RIGHT);

	if (sptr->status == InTransit || sptr->status == LeavingOrbit) {
		i = sptr->getStarID();

		if (_game->isStarExplored(i, _activePlayer)) {
			idx = HSTR_SHIPINFO_DESTINATION_KNOWN;
			str = gameLang->hstrings(idx);
			buf.printf(str, _game->_starSystems[i].name);
			str = buf.c_str();
		} else {
			idx = HSTR_SHIPINFO_DESTINATION_UNKNOWN;
			str = gameLang->hstrings(idx);
		}

		newInfo->setFont(FONTSIZE_SMALL,
			FONT_COLOR_FLEETLIST_SHIPINFO_DESTINATION, 2);
		newInfo->appendText(str, 0, newInfo->height(),
			FLEETLIST_SHIPINFO_WIDTH);
	} else {
		newInfo->appendText("\n", 0, newInfo->height(),
			FLEETLIST_SHIPINFO_WIDTH);
	}

	y = newInfo->height();
	newInfo->setFont(FONTSIZE_SMALL, FONT_COLOR_HELP);
	newInfo->appendText(gameLang->hstrings(HSTR_SHIPINFO_WEAPONS), 0, y,
		FLEETLIST_SHIPINFO_WIDTH);
	newInfo->appendText(gameLang->hstrings(HSTR_SHIPINFO_SPECIALS),
		FLEETLIST_SHIPINFO_COL2, y, FLEETLIST_SHIPINFO_WIDTH);

	y = newInfo->height();
	newInfo->setFont(FONTSIZE_SMALLER, FONT_COLOR_HELP);

	for (i = 0, count = 0; i < MAX_SHIP_WEAPONS; i++) {
		wpn = sptr->design.weapons + i;

		if (wpn->type <= 0 || wpn->maxCount <= 0) {
			continue;
		}

		if (wpn->maxCount == 1) {
			idx = TNAME_WEAPONTYPE_NONE;
		} else {
			idx = TNAME_WEAPONTYPE_NONE_PLURAL;
		}

		str = gameLang->techname(idx + wpn->type);
		buf.printf("%d %s (%s)", wpn->maxCount, str, wpn->arcAbbr());
		newInfo->appendText(buf.c_str(), 5, y + count * lineheight,
			FLEETLIST_SHIPINFO_WIDTH);
		count++;
	}

	if (!count) {
		str = gameLang->techname(HSTR_SHIPINFO_NONE);
		newInfo->appendText(str, 5, y, FLEETLIST_SHIPINFO_WIDTH);
	}

	for (i = 1, count = 0; i < MAX_SHIP_SPECIALS; i++) {
		if (!sptr->hasSpecial(i)) {
			continue;
		}

		if (sptr->isSpecialDamaged(i)) {
			newInfo->setFont(FONTSIZE_SMALLER,
				FONT_COLOR_FLEETLIST_SPECDAMAGE);
		} else {
			newInfo->setFont(FONTSIZE_SMALLER, FONT_COLOR_HELP);
		}

		str = gameLang->techname(TNAME_SPECIALTYPE_NONE + i);
		newInfo->appendText(str, FLEETLIST_SHIPINFO_COL2 + 15,
			y + count * lineheight, FLEETLIST_SHIPINFO_WIDTH);
		count++;
	}

	if (!count) {
		str = gameLang->techname(HSTR_SHIPINFO_NONE);
		newInfo->setFont(FONTSIZE_SMALLER, FONT_COLOR_HELP);
		newInfo->appendText(str, FLEETLIST_SHIPINFO_COL2 + 15,
			y, FLEETLIST_SHIPINFO_WIDTH);
	}

	if (sptr->officer >= 0) {
		idx = sptr->officer;
		x2 = FLEETLIST_SHIPINFO_WIDTH - 84;
		y = 38;
		eta = _game->_leaders[idx].eta;
		fnt = gameFonts->getFont(FONTSIZE_TINY);
		newInfo->setFont(FONTSIZE_TINY, FONT_COLOR_HELP);
		buf.printf("%s %s,", _game->_leaders[idx].rank(),
			_game->_leaders[idx].name);
		newInfo->appendText(buf.c_str(), 0, y, x2, ALIGN_RIGHT);
		y += fnt->height() + 1;
		str = gameLang->misctext(TXT_MISC_JIMTEXT2,
			JIM2_DEFINITE_ARTICLE);
		buf.printf("%s%s", str, _game->_leaders[idx].title);
		newInfo->appendText(buf.c_str(), 0, y, x2, ALIGN_RIGHT);
		sprite = new GuiSprite(FLEETLIST_ARCHIVE,
			ASSET_FLEET_LEADER_IMAGES+_game->_leaders[idx].picture,
			_bg->palette(), 0, 0, 0);
	}

	clearShipInfo();
	_infoOffset = 0;
	_shipInfo = newInfo;
	_shipOfficer = sprite;
	_officerETA = eta;
}

void FleetListView::redrawShipInfo(unsigned curtick) {
	unsigned eta, x, y, w, h, offset = _infoOffset;
	const char *str;
	TextLayout *info = _shipInfo;
	GuiSprite *img = _shipOfficer;

	eta = _officerETA;

	if (!info) {
		return;
	}

	if (img) {
		const uint8_t *frm;

		x = 16 + FLEETLIST_SHIPINFO_WIDTH - img->width();
		y = 284;
		w = img->width();
		h = img->height();
		img->redraw(x, y, curtick);

		if (eta) {
			frm = fleetlistOfficerRedFrame;
		} else {
			frm = fleetlistOfficerBlueFrame;
		}

		drawFrame(x - 2, y - 2, w + 4, h + 4, frm);

		if (eta) {
			StringBuffer buf;
			Font *fnt;

			fnt = gameFonts->getFont(FONTSIZE_SMALL);
			x += w / 2;
			y += 1 + (h - fnt->height()) / 2;
			str = gameLang->hstrings(HSTR_FLEET_OFFICER_ETA);
			buf.printf(str, eta);
			w = fnt->textWidth(buf.c_str(), 2);
			x -= w / 2;

			// double outline
			fnt->renderText(x + 1, y + 1,
				FONT_COLOR_FLEETLIST_SPECDAMAGE, buf.c_str(),
				OUTLINE_FULL, 2);
			fnt->renderText(x, y, FONT_COLOR_FLEETLIST_SPECDAMAGE,
				buf.c_str(), OUTLINE_FULL, 2);
			fillRect(x - 1, y - 4, w + 3, 1, 0xc4, 0, 0);
			fillRect(x - 1, y - 3, w + 3, 1, 0x50, 0x0c, 0x0c);
			y += fnt->height();
			fillRect(x - 1, y, w + 3, 1, 0x50, 0x0c, 0x0c);
			fillRect(x - 1, y + 1, w + 3, 1, 0xc4, 0, 0);
		}
	}

	info->redraw(18, 287 + offset, curtick);
}

void FleetListView::redraw(unsigned curtick) {
	clearScreen();
	_bg->draw(0, 0);

	redrawWidgets(0, 0, curtick);
	redrawShipInfo(curtick);
	redrawWindows(curtick);
}

void FleetListView::open(void) {
	_grid->highlight(-1);
	shipHighlightChanged(0, 0, 0);
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

void FleetListView::clickPrevFleet(int x, int y, int arg) STUB(this)

void FleetListView::clickNextFleet(int x, int y, int arg) STUB(this)

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

void FleetListView::clickRelocate(int x, int y, int arg) STUB(this)

void FleetListView::clickScrap(int x, int y, int arg) STUB(this)

void FleetListView::clickLeaders(int x, int y, int arg) STUB(this)

void FleetListView::clickReturn(int x, int y, int arg) {
	exitView();
}

void fleetSummary(StringBuffer &buf, const Fleet &f) {
	unsigned idx, i, tmp;
	const char *str;
	int shipTypes[] = {COLONY_SHIP, TRANSPORT_SHIP, OUTPOST_SHIP, -1};
	unsigned shipTypeNames[] = {
		HSTR_SHIPCLASS_COLONY, HSTR_SHIPCLASS_TRANSPORT,
		HSTR_SHIPCLASS_OUTPOST
	};

	idx = f.getOwner();
	str = f.getRace();

	if (idx > MAX_PLAYERS) {
		buf = str;
		buf.toUpper();
		return;
	}

	buf.printf(gameLang->hstrings(HSTR_FLEET_RACE_DETAIL_FMT), str);
	buf.toUpper();

	for (i = 0; shipTypes[i] >= 0; i++) {
		tmp = f.shipTypeCount(shipTypes[i]);

		if (!tmp) {
			continue;
		}

		str = gameLang->hstrings(shipTypeNames[i] + (tmp > 1 ? 1 : 0));
		buf.append_printf(str, (int)tmp);
	}

	for (i = 0; i < MAX_COMBAT_SHIP_CLASSES; i++) {
		tmp = f.combatClassCount(i);

		if (!tmp) {
			continue;
		} else if (tmp == 1) {
			idx = TNAME_SHIPCLASS_FRIGATE;
		} else {
			idx = TNAME_SHIPCLASS_FRIGATE_PLURAL;
		}

		str = gameLang->techname(idx + i);
		buf.append_printf("%u %s, ", tmp, str);
	}

	str = buf.c_str();
	i = buf.length() - 1;
	for (; i > 0 && (str[i] == ',' || str[i] == ' '); i--);
	buf.truncate(i + 1);
}
