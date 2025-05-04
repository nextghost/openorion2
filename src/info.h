/*
 * This file is part of OpenOrion2
 * Copyright (C) 2024 Martin Doucha
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

#ifndef INFO_H_
#define INFO_H_

#include "lbx.h"
#include "gui.h"
#include "gamestate.h"

#define INFO_PANEL_COUNT 5

class HistoryGraphWidget : public Widget {
private:
	const GameState *_game;
	int _activePlayer;

public:
	HistoryGraphWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, const GameState *game, int _activePlayer);
	~HistoryGraphWidget(void);

	void redraw(int x, int y, unsigned curtick);
};

class TechReviewWidget : public Widget {
private:
	const GameState *_game;
	int _activePlayer;

public:
	TechReviewWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, const GameState *game, int _activePlayer);
	~TechReviewWidget(void);

	void redraw(int x, int y, unsigned curtick);
};

class RaceInfoWidget : public CompositeWidget {
private:
	const GameState *_game;
	int _activePlayer;
	unsigned _page;

	void initWidgets(void);

protected:
	unsigned playerCount(void) const;

public:
	RaceInfoWidget(unsigned x, unsigned y, unsigned width, unsigned height,
		const GameState *game, int _activePlayer);
	~RaceInfoWidget(void);

	void nextPage(int x, int y, int arg);

	void redraw(int x, int y, unsigned curtick);
};

class TurnSummaryWidget : public Widget {
private:
	const GameState *_game;
	int _activePlayer;

public:
	TurnSummaryWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, const GameState *game, int _activePlayer);
	~TurnSummaryWidget(void);

	void redraw(int x, int y, unsigned curtick);
};

class DocsWidget : public Widget {
public:
	DocsWidget(unsigned x, unsigned y, unsigned width,
		unsigned height);
	~DocsWidget(void);

	void redraw(int x, int y, unsigned curtick);
};

class InfoView : public GuiView {
private:
	ImageAsset _bg;
	BitmapAsset _vbar, _barLabel, _barMask, _barFoot;
	GameState *_game;
	int _activePlayer;
	ChoiceWidget *_panelChoice;
	Widget *_panels[INFO_PANEL_COUNT];

	void initWidgets(void);

protected:
	void changePanel(int x, int y, int arg);

public:
	InfoView(GameState *game, int activePlayer);
	~InfoView(void);

	void redraw(unsigned curtick);

	void clickReturn(int x, int y, int arg);
};

#endif
