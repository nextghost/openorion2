/*
 * This file is part of OpenOrion2
 * Copyright (C) 2023 Martin Doucha
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
#include "lang.h"
#include "colony.h"

#define RACEICON_ARCHIVE "raceicon.lbx"
#define ASSET_FARMER 1
#define ASSET_WORKER 3
#define ASSET_SCIENTIST 5
#define ASSET_PRISONER 12
#define ASSET_ANDROID 169
#define ASSET_NATIVE 170
#define RACE_ASSET_COUNT 13
#define JOB_ASSET_COUNT 3

#define COLONY2_ARCHIVE "colony2.lbx"
#define ASSET_COLONY2_PAL 50

static const unsigned job_sprite_assets[JOB_ASSET_COUNT] = {
	ASSET_FARMER, ASSET_WORKER, ASSET_SCIENTIST
};

ColonistPickerWidget::ColonistPickerWidget(unsigned x, unsigned y,
	unsigned width, unsigned height, const Colony *cptr, unsigned job,
	const Player *players, unsigned playerCount) :
	Widget(x, y, width, height), _spriteWidth(1), _job(job),
	_colonistCount(0), _colony(cptr) {

	unsigned i, offset;
	const uint8_t *pal;
	ImageAsset palImg = gameAssets->getImage(COLONY2_ARCHIVE,
		ASSET_COLONY2_PAL);

	pal = palImg->palette();

	for (i = 0; i < playerCount; i++) {
		offset = players[i].picture * RACE_ASSET_COUNT;
		_sprites[i] = gameAssets->getImage(RACEICON_ARCHIVE,
			offset + job_sprite_assets[job], pal);
		_spriteWidth = MAX(_spriteWidth, _sprites[i]->width());
		_sprites[MAX_RACES + i] = gameAssets->getImage(RACEICON_ARCHIVE,
			offset + ASSET_PRISONER, pal);
		_spriteWidth = MAX(_spriteWidth,
			_sprites[MAX_RACES + i]->width());
	}

	_sprites[ColonistRace::ANDROID] = gameAssets->getImage(RACEICON_ARCHIVE,
		ASSET_ANDROID, pal);
	_spriteWidth = MAX(_spriteWidth,
		_sprites[ColonistRace::ANDROID]->width());
	_sprites[ColonistRace::NATIVE] = gameAssets->getImage(RACEICON_ARCHIVE,
		ASSET_NATIVE, pal);
	_spriteWidth = MAX(_spriteWidth,
		_sprites[ColonistRace::NATIVE]->width());
	update();
}

unsigned ColonistPickerWidget::spriteSpacing(unsigned count) const {
	unsigned ret = width();

	if (count < 2 || ret < _spriteWidth + count) {
		return 1;
	}

	ret -= _spriteWidth;
	ret /= count - 1;
	return MIN(ret, _spriteWidth + 2);
}

void ColonistPickerWidget::update(void) {
	unsigned i, idx, flags, count;
	unsigned goffsets[MAX_COLONIST_GROUPS] = {0};
	int gindex[MAX_POPULATION];

	_colonistCount = 0;
	memset(_groupOffsets, 0, MAX_COLONIST_GROUPS * sizeof(unsigned));

	if (!_colony) {
		return;
	}

	for (i = 0; i < _colony->population; i++) {
		flags = _colony->colonists[i].flags;
		gindex[i] = -1;

		if (!(flags & ColonistFlags::WORKING)) {
			continue;
		}

		if (_colony->colonists[i].job != _job) {
			continue;
		}

		idx = (flags & ColonistFlags::PRISONER) ? MAX_RACES : 0;
		idx += _colony->colonists[i].race;
		gindex[i] = idx;
		goffsets[idx]++;
	}

	for (i = 0, count = 0; i < MAX_COLONIST_GROUPS; i++) {
		idx = count;
		count += goffsets[i];
		goffsets[i] = idx;
	}

	for (i = 0; i < _colony->population; i++) {
		if (gindex[i] >= 0) {
			_colonistIndex[goffsets[gindex[i]]++] = i;
		}
	}

	_colonistCount = count;
	_groupOffsets[0] = 0;

	for (i = 0; i < MAX_COLONIST_GROUPS - 1; i++) {
		idx = _groupOffsets[i];
		_groupOffsets[i + 1] = idx > goffsets[i] ? idx : goffsets[i];
	}
}

void ColonistPickerWidget::setColony(const Colony *cptr) {
	AutoMutex am(_mutex);

	_colonistCount = 0;
	memset(_groupOffsets, 0, MAX_COLONIST_GROUPS * sizeof(unsigned));
	_colony = cptr;
	update();
}

int *ColonistPickerWidget::pickColonists(int x) const {
	unsigned i, pos, count = _colonistCount;
	int *ret;

	if (!count) {
		return NULL;
	}

	x -= getX();
	pos = (x > 0) ? x / spriteSpacing(count) : 0;
	pos = (pos >= count) ? count - 1 : pos;
	for (i = 1; i < MAX_COLONIST_GROUPS && pos >= _groupOffsets[i]; i++);
	count = (i < MAX_COLONIST_GROUPS) ? _groupOffsets[i] : count;
	count -= pos;
	ret = new int[count + 1];
	memcpy(ret, _colonistIndex + pos, count * sizeof(int));
	ret[count] = -1;
	return ret;
}

const ImageAsset &ColonistPickerWidget::getSprite(unsigned colonist_id) const {
	unsigned idx;
	const Colonist *ptr;

	if (colonist_id >= _colony->population) {
		throw std::out_of_range("Invalid colonist ID");
	}

	ptr = _colony->colonists + colonist_id;

	if (!(ptr->flags & ColonistFlags::WORKING)) {
		throw std::runtime_error("Invalid colonist ID");
	}

	if (ptr->job != _job) {
		throw std::runtime_error("Colonist job mismatch");
	}

	idx = (ptr->flags & ColonistFlags::PRISONER) ? MAX_RACES : 0;
	return _sprites[idx + ptr->race];
}

void ColonistPickerWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, idx, count, spacing;
	int x2;
	const Colonist *cptr;
	AutoMutex am(_mutex);

	if (!_colony) {
		return;
	}

	x += getX();
	y += getY();

	count = _colonistCount;
	spacing = spriteSpacing(count);

	for (i = 0, x2 = x; i < count; i++) {
		cptr = _colony->colonists + _colonistIndex[i];
		idx = (cptr->flags & ColonistFlags::PRISONER) ? MAX_RACES : 0;
		idx += cptr->race;
		_sprites[idx]->draw(x2, y);
		x2 += spacing;
	}

	if (_job == ColonistJob::FARMER && !_colony->food_per_farmer) {
		Font *fnt = gameFonts->getFont(FONTSIZE_MEDIUM);

		if (fnt->height() < height()) {
			y += (height() - fnt->height()) / 2;
		}

		fnt->centerText(x + width() / 2, y, FONT_COLOR_COLONY_LIST,
			gameLang->estrings(ESTR_NO_FARMING), OUTLINE_SHADOW, 2);
	}
}
