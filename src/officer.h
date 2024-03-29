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

#ifndef OFFICER_H_
#define OFFICER_H_

#include "ships.h"

#define MAX_HIRED_LEADERS 4

class LeaderSkillsWidget : public Widget {
private:
	GuiView *_parent;
	const GameState *_game;
	ImageAsset _skillImg[MAX_SKILLS];
	int _leaderID;
	unsigned _drawOffset, _fontColor, _skills[MAX_SKILLS], _skillCount;

public:
	LeaderSkillsWidget(GuiView *parent, unsigned x, unsigned y,
		 unsigned width, unsigned height, const GameState *game,
		 unsigned drawOffset = 0);
	~LeaderSkillsWidget(void);

	void setLeader(int id);
	void setFontColor(unsigned color);

	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class LeaderListView : public GuiView {
private:
	GameState *_game;
	ChoiceWidget *_panelChoice;
	LeaderSkillsWidget *_leaderSlots[MAX_HIRED_LEADERS];
	StarSystemWidget *_starSystem;
	ShipGridWidget *_grid;
	ScrollBarWidget *_scroll;
	Widget *_scrollUp, *_scrollDown;
	GalaxyMinimapWidget *_minimap;
	LabelWidget *_minimapLabel, *_selectionLabel;
	int _activePlayer, _curLeader, _selLeader, _curStar, _scrollgrab;
	ImageAsset _bg, _leaderImg[LEADER_COUNT], _leaderDarkImg[LEADER_COUNT];
	unsigned _admins[MAX_HIRED_LEADERS], _captains[MAX_HIRED_LEADERS];
	StringBuffer _adminNames[MAX_HIRED_LEADERS];
	StringBuffer _captainNames[MAX_HIRED_LEADERS];
	unsigned _adminCount, _captainCount;
	Fleet *_curFleet;

	void initWidgets(void);

protected:
	void fleetHighlightChanged(int x, int y, int arg);
	void fleetSelectionChanged(int x, int y, int arg);
	void shipHighlightChanged(int x, int y, int arg);
	void shipSelectionChanged(int x, int y, int arg);
	void starHighlightChanged(int x, int y, int arg);
	void starSelectionChanged(int x, int y, int arg);

	void handleBeginScroll(int x, int y, int arg);
	void handleScroll(int x, int y, int arg);
	void handleEndScroll(int x, int y, int arg);
	void updateScrollbar(void);

	int getLeaderID(int slot) const;
	const char *getRankedName(int slot) const;

	void showSelectionHelp(int x, int y, int arg);
	void showSlotHelp(int x, int y, int arg);

	void selectOfficerLocation(int x, int y, int arg);
	void cancelSelect(int x, int y, int arg);
	void assignOfficer(int x, int y, int arg);

	void askAssignOfficer(void);
	void slotStateChanged(int slot);

public:
	LeaderListView(GameState *game, int activePlayer);

	void redraw(unsigned curtick);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned buttons);

	void showHelp(int x, int y, int arg);
	void changePanel(int x, int y, int arg);
	void highlightSlot(int x, int y, int arg);
	void selectSlot(int x, int y, int arg);
	void clickSelectPrevious(int x, int y, int arg);
	void clickSelectNext(int x, int y, int arg);
	void clickHireButton(int x, int y, int arg);
	void clickPoolButton(int x, int y, int arg);
	void clickDismissButton(int x, int y, int arg);
	void clickReturn(int x, int y, int arg);
};

class HireLeaderWindow : public GuiWindow {
private:
	GameState *_game;
	int _activePlayer;
	unsigned _leaderID;
	ImageAsset _bg, _leaderImg;
	StringBuffer _fullname;
	TextLayout _text;

	void initWidgets(int can_hire);

	void clickHire(int x, int y, int arg);

public:
	HireLeaderWindow(GuiView *parent, GameState *game, int activePlayer,
		unsigned leader_id, int newOffer,
		unsigned flags = WINDOW_MOVABLE | WINDOW_MODAL);
	~HireLeaderWindow(void);

	void redraw(unsigned curtick);

	void showHelp(int x, int y, int arg);
};

#endif
