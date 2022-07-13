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
#include "ships.h"

#define SHIPSPRITE_ARCHIVE "ships.lbx"
#define PALSPRITE_ANTARAN 413
#define PALSPRITE_DRAGON 414
#define PALSPRITE_EEL 415
#define PALSPRITE_CRYSTAL 416
#define PALSPRITE_HYDRA 417
#define PALSPRITE_AMOEBA 418
#define PALSPRITE_GUARDIAN 419

static const unsigned monster_palettes[MAX_SHIPTYPES_MONSTER] = {
	PALSPRITE_AMOEBA, PALSPRITE_CRYSTAL, PALSPRITE_DRAGON, PALSPRITE_EEL,
	PALSPRITE_HYDRA
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
