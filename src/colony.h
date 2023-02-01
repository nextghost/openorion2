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

#ifndef COLONY_H_
#define COLONY_H_

#include "lbx.h"
#include "gui.h"
#include "gamestate.h"

#define MAX_COLONIST_GROUPS (MAX_RACES + MAX_PLAYERS)

class ColonistPickerWidget : public Widget {
private:
	ImageAsset _sprites[MAX_COLONIST_GROUPS];
	unsigned _spriteWidth, _job, _groupOffsets[MAX_COLONIST_GROUPS];
	unsigned _colonistCount, _colonistIndex[MAX_POPULATION];
	const Colony *_colony;
	Mutex _mutex;

protected:
	unsigned spriteSpacing(unsigned count) const;

public:
	ColonistPickerWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, const Colony *cptr, unsigned job,
		const Player *players, unsigned playerCount);

	// refresh colonist index after colony changes
	void update(void);

	void setColony(const Colony *cptr);

	// returns freshly allocated array of colonist IDs or NULL
	// colonist array is terminated by negative value
	// caller must change their presence flag to 0 and call update()
	int *pickColonists(int x) const;

	// return sprite for _colony->colonists[colonist_id]
	const ImageAsset &getSprite(unsigned colonist_id) const;

	void redraw(int x, int y, unsigned curtick);
};

#endif
