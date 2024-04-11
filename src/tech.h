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

#ifndef TECH_H_
#define TECH_H_

#include "lbx.h"
#include "gui.h"
#include "gamestate.h"

#define MAX_RESEARCH_CHOICES 4

struct ResearchChoice {
	int cost;
	int research_all; // There is no choice, all techs will be unlocked
	Technology choices[MAX_RESEARCH_CHOICES];
};

class TechListWidget : public Widget {
public:
	struct TechListItem {
		Technology tech_id;
		unsigned color;
		char *name;
	};

private:
	struct TechListGroup {
		char *title;
		unsigned itemCount, height, color;
		TechListItem *items;

		TechListGroup(void);
		~TechListGroup(void);

		void clear(void);
	};

	GuiCallback _onHighlightItem, _onSelectItem, _onExamineItem;
	int _curGroup, _curItem, _selGroup, _selItem;
	unsigned _groupCount, _maxGroups, _curPage, _pageCount, _maxPages;
	unsigned _titleFont, _itemFont;
	unsigned *_pages;
	TechListGroup **_groups;

protected:
	int updateHighlight(int x, int y);

public:
	TechListWidget(unsigned x, unsigned y, unsigned width, unsigned height,
		unsigned titleFont, unsigned itemFont);
	~TechListWidget(void);

	void clear(void);
	void addGroup(const char *title, unsigned color,
		const TechListItem *items, unsigned itemCount);

	void highlightItem(int group, int item);
	int highlightedGroup(void) const;
	int highlightedItem(void) const;
	Technology highlightedTechID(void) const;

	void selectItem(int group, int item);
	int selectedGroup(void) const;
	int selectedItem(void) const;
	Technology selectedTechID(void) const;

	unsigned currentPage(void) const;
	unsigned pageCount(void) const;
	void setPage(unsigned page);
	void previousPage(void);
	void nextPage(void);

	void setItemHighlightCallback(const GuiCallback &callback);
	void setItemSelectionCallback(const GuiCallback &callback);
	void setItemExamineCallback(const GuiCallback &callback);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class ResearchSelectWidget : public Widget {
private:
	GuiView *_parent;
	GameState *_game;
	GuiCallback _onHighlightChange, _onSelectionChange;
	int _activePlayer, _highlight, _selection;
	ResearchTopic _topic;
	Technology _choices[MAX_RESEARCH_CHOICES];
	unsigned _choiceCount, _startTick;

protected:
	unsigned findChoice(int y) const;

public:
	ResearchSelectWidget(GuiView *parent, int x, int y, unsigned width,
		unsigned height, GameState *game, int activePlayer,
		unsigned area);
	~ResearchSelectWidget(void);

	int isSelected(void) const;
	int isHighlighted(void) const;
	unsigned researchCost(int full = 0) const;

	void setHighlightCallback(const GuiCallback &callback);
	void setSelectCallback(const GuiCallback &callback);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseOut(int x, int y, unsigned buttons);
	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class ResearchSelectWindow : public GuiWindow {
private:
	ImageAsset _bg;
	GameState *_game;
	int _activePlayer;
	ResearchSelectWidget *_techChoices[MAX_RESEARCH_AREAS];
	LabelWidget *_costLabels[MAX_RESEARCH_AREAS];

	void initWidgets(int allow_cancel);

protected:
	void techHighlightChanged(int x, int y, int arg);
	void techSelected(int x, int y, int arg);

public:
	ResearchSelectWindow(GuiView *parent, GameState *game,
		int activePlayer, int allow_cancel);
	~ResearchSelectWindow(void);

	void clickTechArea(int x, int y, int arg);

	void redraw(unsigned curtick);
};

class ResearchListWindow : public GuiWindow {
private:
	ImageAsset _bg;
	GameState *_game;
	unsigned _topicOffset;
	int _activePlayer;
	ResearchArea _area;
	Widget *_buttonUp, *_buttonDown;
	TechListWidget *_list;

	void initWidgets(void);
	void initList(void);
	void enablePageButtons(void);

protected:
	void showTechHelp(int x, int y, int arg);
	void clearHighlight(int x, int y, int arg);
	void clearSelection(int x, int y, int arg);

public:
	ResearchListWindow(GuiView *parent, GameState *game, int activePlayer,
		ResearchArea area,
		unsigned flags = WINDOW_MODAL | WINDOW_MOVABLE);
	~ResearchListWindow(void);

	void previousPage(int x, int y, int arg);
	void nextPage(int x, int y, int arg);

	void redraw(unsigned curtick);
};

extern const ResearchChoice research_choices[MAX_RESEARCH_TOPICS];

int isHyperTopic(unsigned topic);

#endif
