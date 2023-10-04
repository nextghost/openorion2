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
#include "screen.h"
#include "guimisc.h"
#include "lang.h"
#include "officer.h"

#define LEADER_LIST_ARCHIVE "officer.lbx"
#define ASSET_LEADERLIST_BG 0
#define ASSET_LEADERLIST_STARSYSTEM_BG 2
#define ASSET_LEADERLIST_ADMINS_PANEL 3
#define ASSET_LEADERLIST_CAPTAINS_PANEL 4
#define ASSET_LEADERLIST_SCROLL_UP_BUTTON 5
#define ASSET_LEADERLIST_SCROLL_DOWN_BUTTON 6
#define ASSET_LEADERLIST_PREV_BUTTON 7
#define ASSET_LEADERLIST_NEXT_BUTTON 8
#define ASSET_LEADERLIST_HIRE_BUTTON 9
#define ASSET_LEADERLIST_POOL_BUTTON 10
#define ASSET_LEADERLIST_DISMISS_BUTTON 11
#define ASSET_LEADERLIST_RETURN_BUTTON 12
#define ASSET_LEADERLIST_SLOT_SELECTED 14
#define ASSET_LEADERLIST_SLOT_HIGHLIGHTED 15
#define ASSET_LEADER_PORTRAITS 21
#define ASSET_LEADERLIST_SKILLS 88
#define ASSET_LEADERLIST_SHIP_IMAGES 115
#define ASSET_LEADERLIST_STAR_IMAGES 130
#define ASSET_LEADER_DARK_PORTRAITS 143
#define ASSET_SKILL_ICONS 88

#define LEADER_LIST_SLOT_X 7
#define LEADER_LIST_PORTRAIT_X 13
#define LEADER_LIST_FIRST_ROW 38
#define LEADER_LIST_ROW_DIST 109
#define LEADER_LIST_SLOT_WIDTH 85
#define LEADER_LIST_SLOT_HEIGHT 105
#define LEADER_LIST_INFO_X 92
#define LEADER_LIST_INFO_WIDTH 204
#define LEADER_LIST_SCROLL_WIDTH 6

static const int leaderListMinimapStarColors[MAX_PLAYERS + 1] = {
	FONT_COLOR_STAR_RED1, FONT_COLOR_STAR_YELLOW1, FONT_COLOR_STAR_GREEN1,
	FONT_COLOR_LEADERLIST_STAR_SILVER, FONT_COLOR_STAR_BLUE2,
	FONT_COLOR_STAR_BROWN1, FONT_COLOR_STAR_PURPLE1,
	FONT_COLOR_STAR_ORANGE1, FONT_COLOR_LEADERLIST_STAR_NEUTRAL
};

static const uint8_t leaderListScrollTexture[LEADER_LIST_SCROLL_WIDTH * 3] = {
	RGB(0x3c3cd4), RGB(0x1c1ca4), RGB(0x1c1ca4), RGB(0x1c1ca4),
	RGB(0x1c1ca4), RGB(0x00006c),
};

static const char *skillFormatStrings[MAX_SKILL_TYPES][MAX_COMMON_SKILLS] = {
	{
		"%d%%", "%+d", "%+d", "%+dBC", "%+dBC", "%+d", "%+d", "%+d%%",
		"%+d%%", "%+d%%"
	},
	{
		"%+d%%", "%+d", "%+d", "%+d", "%+d", "%+d", "%+d", "%+d"
	},
	{
		"%+d%%", "%+d%%", "%+d%%", "%+d", "%+d%%", "%+d%%", "%+d%%",
		"%+d%%", "%+d"
	}
};

LeaderSkillsWidget::LeaderSkillsWidget(GuiView *parent, unsigned x, unsigned y,
	unsigned width, unsigned height, const GameState *game,
	unsigned drawOffset) : Widget(x, y, width, height), _parent(parent),
	_game(game), _leaderID(-1), _drawOffset(drawOffset),
	_fontColor(FONT_COLOR_LEADERLIST_NORMAL), _skillCount(0) {

	ImageAsset palImg;
	unsigned i;
	const uint8_t *pal;

	if (height < drawOffset + 85) {
		throw std::out_of_range("Widget height is too small");
	}

	palImg = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = palImg->palette();

	for (i = 0; i < MAX_SKILLS; i++) {
		_skillImg[i] = gameAssets->getImage(LEADER_LIST_ARCHIVE,
			ASSET_LEADERLIST_SKILLS + i, pal);
	}
}

LeaderSkillsWidget::~LeaderSkillsWidget(void) {

}

void LeaderSkillsWidget::setLeader(int id) {
	unsigned i, skillbase, skillcount;
	const Leader *ptr;

	if (id >= LEADER_COUNT) {
		throw std::out_of_range("Invalid leader ID");
	}

	_skillCount = 0;
	_leaderID = id;

	if (_leaderID < 0) {
		return;
	}

	ptr = _game->_leaders + _leaderID;

	if (ptr->type == LEADER_TYPE_CAPTAIN) {
		skillbase = CAPTAIN_SKILLS_TYPE;
		skillcount = MAX_CAPTAIN_SKILLS;
	} else {
		skillbase = ADMIN_SKILLS_TYPE;
		skillcount = MAX_ADMIN_SKILLS;
	}

	for (i = 0; i < skillcount; i++) {
		if (ptr->hasSkill(skillbase + i)) {
			_skills[_skillCount++] = skillbase + i;
		}
	}

	skillbase = COMMON_SKILLS_TYPE;

	for (i = 0; i < MAX_COMMON_SKILLS; i++) {
		if (ptr->hasSkill(skillbase + i)) {
			_skills[_skillCount++] = skillbase + i;
		}
	}
}

void LeaderSkillsWidget::setFontColor(unsigned color) {
	_fontColor = color;
}

void LeaderSkillsWidget::handleMouseUp(int x, int y, unsigned button) {
	int minY = getY() + _drawOffset;
	int rowHeight = _skillImg[0]->height() + 1;

	if (_leaderID >= 0 && button == MBUTTON_RIGHT && y >= minY &&
		y < minY + (int)_skillCount * rowHeight) {

		unsigned skill, idx;
		const Leader *ptr = _game->_leaders + _leaderID;
		StringBuffer namebuf, buf;

		namebuf.printf("%s %s", ptr->rank(), ptr->name);
		idx = HSTR_OFFICER_DEFINITE_ARTICLE;
		namebuf.append(gameLang->hstrings(idx));
		namebuf.append(gameLang->officerTitle(_leaderID));
		namebuf.append(",");
		skill = _skills[(y - minY) / rowHeight];
		idx = ptr->skillNum(skill);
		buf.printf(gameLang->skilldesc(idx), namebuf.c_str(),
			abs(ptr->skillBonus(skill)));
		new MessageBoxWindow(_parent, gameLang->skillname(idx),
			buf.c_str());
		return;
	}

	Widget::handleMouseUp(x, y, button);
}

void LeaderSkillsWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, idx, x2, level;
	Font *fnt;
	const Leader *ptr;
	const char *str;

	if (_leaderID < 0) {
		return;
	}

	StringBuffer buf;

	ptr = _game->_leaders + _leaderID;
	x += getX();
	y += getY() + _drawOffset;
	fnt = gameFonts->getFont(FONTSIZE_MEDIUM);

	for (i = 0; i < _skillCount; i++) {
		idx = _skills[i];
		level = ptr->hasSkill(idx);
		str = skillFormatStrings[SKILLTYPE(idx)][idx & SKILLCODE_MASK];
		buf.printf(str, ptr->skillBonus(idx));
		str = Leader::skillName(idx, level > 1);
		idx = Leader::skillNum(idx);
		_skillImg[idx]->draw(x + 2, y);
		fnt->renderText(x + 24, y + 4, _fontColor, str, OUTLINE_FULL);
		x2 = x + width() - 3;
		x2 -= fnt->textWidth(buf.c_str());
		fnt->renderText(x2, y + 4, _fontColor, buf.c_str(),
			OUTLINE_FULL);
		y += _skillImg[idx]->height() + 1;
	}
}

LeaderListView::LeaderListView(GameState *game, int activePlayer) :
	_game(game), _panelChoice(NULL), _starSystem(NULL), _grid(NULL),
	_scroll(NULL), _scrollUp(NULL), _scrollDown(NULL), _minimap(NULL),
	_minimapLabel(NULL), _selectionLabel(NULL),
	_activePlayer(activePlayer), _curLeader(-1), _selLeader(-1),
	_curStar(-1), _scrollgrab(0), _adminCount(0), _captainCount(0),
	_curFleet(NULL) {

	ImageAsset palImg;
	unsigned i, type;
	const uint8_t *pal;

	for (i = 0; i < MAX_HIRED_LEADERS; i++) {
		_leaderSlots[i] = NULL;
	}

	palImg = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	pal = palImg->palette();
	_bg = gameAssets->getImage(LEADER_LIST_ARCHIVE, ASSET_LEADERLIST_BG,
		pal);

	for (i = 0; i < LEADER_COUNT; i++) {
		_leaderImg[i] = gameAssets->getImage(LEADER_LIST_ARCHIVE,
			ASSET_LEADER_PORTRAITS + i, pal);
		_leaderDarkImg[i] = gameAssets->getImage(LEADER_LIST_ARCHIVE,
			ASSET_LEADER_DARK_PORTRAITS + i, pal);

		if (_game->_leaders[i].playerIndex != _activePlayer) {
			continue;
		}

		type = _game->_leaders[i].type;

		if (type == LEADER_TYPE_ADMIN &&
			_adminCount < MAX_HIRED_LEADERS) {
			_admins[_adminCount] = i;
			_adminNames[_adminCount].printf("%s %s",
				_game->_leaders[i].rank(),
				_game->_leaders[i].name);
			_adminCount++;
		} else if (type == LEADER_TYPE_CAPTAIN &&
			_captainCount < MAX_HIRED_LEADERS) {
			_captains[_captainCount] = i;
			_captainNames[_captainCount].printf("%s %s",
				_game->_leaders[i].rank(),
				_game->_leaders[i].name);
			_captainCount++;
		}
	}

	i = _game->_players[_activePlayer].homePlayerId;
	_curStar = _game->_planets[i].star;
	initWidgets();
	changePanel(0, 0, 0);
	starSelectionChanged(0, 0, 0);
}

void LeaderListView::initWidgets(void) {
	ImageAsset img;
	unsigned i;
	Widget *w;
	Font *fnt = gameFonts->getFont(FONTSIZE_MEDIUM);
	const uint8_t *pal = _bg->palette();

	for (i = 0; i < MAX_HIRED_LEADERS; i++) {
		w = createWidget(LEADER_LIST_SLOT_X,
			LEADER_LIST_FIRST_ROW + i * LEADER_LIST_ROW_DIST,
			LEADER_LIST_SLOT_WIDTH, LEADER_LIST_SLOT_HEIGHT);
		w->setMouseOverCallback(GuiMethod(*this,
			&LeaderListView::highlightSlot, i));
		w->setMouseOutCallback(GuiMethod(*this,
			&LeaderListView::highlightSlot, -1));
		w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
			&LeaderListView::selectSlot, i));
		w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
			&LeaderListView::selectOfficerLocation, i));

		_leaderSlots[i] = new LeaderSkillsWidget(this,
			LEADER_LIST_INFO_X, LEADER_LIST_FIRST_ROW +
			i * LEADER_LIST_ROW_DIST, LEADER_LIST_INFO_WIDTH,
			LEADER_LIST_SLOT_HEIGHT, _game, fnt->height() - 1);
		addWidget(_leaderSlots[i]);
		_leaderSlots[i]->setMouseOverCallback(GuiMethod(*this,
			&LeaderListView::highlightSlot, i));
		_leaderSlots[i]->setMouseOutCallback(GuiMethod(*this,
			&LeaderListView::highlightSlot, -1));
		_leaderSlots[i]->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &LeaderListView::selectSlot, i));
		_leaderSlots[i]->setMouseUpCallback(MBUTTON_RIGHT,
			GuiMethod(*this, &LeaderListView::showSlotHelp));
	}

	_panelChoice = new ChoiceWidget(7, 10, 296, 23, 2);
	addWidget(_panelChoice);
	_panelChoice->setChoiceButton(0, 0, 0, 150, 23, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_ADMINS_PANEL, pal, 1);
	_panelChoice->setChoiceButton(1, 153, 0, 140, 23, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_CAPTAINS_PANEL, pal, 1);
	_panelChoice->setValueChangeCallback(GuiMethod(*this,
		&LeaderListView::changePanel));
	_panelChoice->button(0)->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &LeaderListView::showHelp,
		HELP_LEADERLIST_ADMINS_BUTTON));
	_panelChoice->button(1)->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &LeaderListView::showHelp,
		HELP_LEADERLIST_CAPTAINS_BUTTON));

	img = gameAssets->getImage(LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_STARSYSTEM_BG, pal);
	_starSystem = new StarSystemWidget(300, 12, 329, 189, _game,
		_activePlayer, _curStar, (Image*)img);
	addWidget(_starSystem);
	_starSystem->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_STAR_SYSTEM));

	img = gameAssets->getImage(LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_SLOT_SELECTED, pal);
	_grid = new ShipGridWidget(this, 301, 18, 3, 5, 0, 0, _game,
		_activePlayer, (Image*)img, NULL, 0);
	addWidget(_grid);
	_grid->setSelectionMode(0);
	_grid->setShipHighlightCallback(GuiMethod(*this,
		&LeaderListView::shipHighlightChanged));
	_grid->setSelectionChangeCallback(GuiMethod(*this,
		&LeaderListView::shipSelectionChanged));
	_grid->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_SHIP_GRID));

	_scroll = new ScrollBarWidget(615, 47, LEADER_LIST_SCROLL_WIDTH, 119,
		3, 3, leaderListScrollTexture);
	addWidget(_scroll);
	_scroll->setBeginScrollCallback(GuiMethod(*this,
		&LeaderListView::handleBeginScroll));
	_scroll->setScrollCallback(GuiMethod(*this,
		&LeaderListView::handleScroll));
	_scroll->setEndScrollCallback(GuiMethod(*this,
		&LeaderListView::handleEndScroll));
	_scroll->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &LeaderListView::showHelp,
		HELP_LEADERLIST_SCROLLBAR));

	_scrollUp = createWidget(613, 22, 10, 19);
	_scrollUp->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollMinus));
	_scrollUp->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_SCROLL_UP_BUTTON, pal, 1);
	_scrollUp->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &LeaderListView::showHelp,
		HELP_LEADERLIST_SCROLLBAR));

	_scrollDown = createWidget(613, 170, 11, 21);
	_scrollDown->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod(*_scroll, &ScrollBarWidget::scrollPlus));
	_scrollDown->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_SCROLL_DOWN_BUTTON, pal, 1);
	_scrollDown->setMouseUpCallback(MBUTTON_RIGHT,
		GuiMethod(*this, &LeaderListView::showHelp,
		HELP_LEADERLIST_SCROLLBAR));

	_minimap = new GalaxyMinimapWidget(306, 235, 318, 169, _game,
		_activePlayer, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_STAR_IMAGES, ASSET_LEADERLIST_SHIP_IMAGES,
		pal);
	addWidget(_minimap);
	_minimap->setStarFilter(SELFILTER_OWNED, SELFILTER_ANY);
	_minimap->selectStar(_curStar);
	_minimap->setFleetHighlightCallback(GuiMethod(*this,
		&LeaderListView::fleetHighlightChanged));
	_minimap->setFleetSelectCallback(GuiMethod(*this,
		&LeaderListView::fleetSelectionChanged));
	_minimap->setStarHighlightCallback(GuiMethod(*this,
		&LeaderListView::starHighlightChanged));
	_minimap->setStarSelectCallback(GuiMethod(*this,
		&LeaderListView::starSelectionChanged));
	_minimap->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showSelectionHelp, 3));

	w = createWidget(313, 441, 74, 27);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickHireButton));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_HIRE_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_HIRE));

	w = createWidget(388, 441, 74, 27);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickPoolButton));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_POOL_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_POOL));

	w = createWidget(463, 441, 74, 27);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickDismissButton));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_DISMISS_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_DISMISS));

	w = createWidget(538, 441, 80, 27);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickReturn));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_RETURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_RETURN));

	w = createWidget(327, 205, 35, 21);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickSelectPrevious));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_PREV_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showSelectionHelp, 1));

	w = createWidget(568, 205, 35, 21);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&LeaderListView::clickSelectNext));
	w->setClickSprite(MBUTTON_LEFT, LEADER_LIST_ARCHIVE,
		ASSET_LEADERLIST_NEXT_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showSelectionHelp, 2));

	_minimapLabel = new LabelWidget(364, 412, 204, 22);
	addWidget(_minimapLabel);
	_minimapLabel->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showHelp, HELP_LEADERLIST_MINIMAP_LABEL));

	_selectionLabel = new LabelWidget(364, 205, 204, 22);
	addWidget(_selectionLabel);
	_selectionLabel->setMouseUpCallback(MBUTTON_RIGHT, GuiMethod(*this,
		&LeaderListView::showSelectionHelp, 0));
}

int LeaderListView::getLeaderID(int slot) const {
	if (slot < 0) {
		return -1;
	}

	if (_panelChoice->value()) {
		return slot >= (int)_captainCount ? -1 : _captains[slot];
	} else {
		return slot >= (int)_adminCount ? -1 : _admins[slot];
	}
}

const char *LeaderListView::getRankedName(int slot) const {
	if (slot < 0) {
		return "";
	}

	if (_panelChoice->value()) {
		if (slot < (int)_captainCount) {
			return _captainNames[slot].c_str();
		}
	} else {
		if (slot < (int)_adminCount) {
			return _adminNames[slot].c_str();
		}
	}

	return "";
}

void LeaderListView::redraw(unsigned curtick) {
	unsigned i, x, y, count, *idlist;
	unsigned color, color2;
	int location;
	const Leader *ptr;
	Font *fnt, *smallfnt;
	const Image *img;
	const char *str;
	StringBuffer buf, *namelist;

	clearScreen();
	_bg->draw(0, 0);
	fnt = gameFonts->getFont(FONTSIZE_MEDIUM);
	smallfnt = gameFonts->getFont(FONTSIZE_SMALL);

	if (_panelChoice->value()) {
		idlist = _captains;
		namelist = _captainNames;
		count = _captainCount;
	} else {
		idlist = _admins;
		namelist = _adminNames;
		count = _adminCount;
	}

	for (i = 0; i < count; i++) {
		ptr = _game->_leaders + idlist[i];
		color = color2 = FONT_COLOR_LEADERLIST_NORMAL;

		if (_curLeader == (int)i || _selLeader == (int)i) {
			color = FONT_COLOR_LEADERLIST_BRIGHT;
			color2 = FONT_COLOR_LEADERLIST_BRIGHT_NAME;
		}

		x = LEADER_LIST_INFO_X;
		y = LEADER_LIST_FIRST_ROW + i * LEADER_LIST_ROW_DIST;
		fnt->centerText(x + LEADER_LIST_INFO_WIDTH / 2, y, color2,
			namelist[i].c_str(), OUTLINE_FULL);

		x = LEADER_LIST_PORTRAIT_X;

		if (ptr->status == LeaderState::ForHire || ptr->eta) {
			img = (const Image*)_leaderDarkImg[ptr->picture];
		} else {
			img = (const Image*)_leaderImg[ptr->picture];
		}

		img->draw(x, y);
		color2 = color;

		switch (ptr->status) {
		case LeaderState::Idle:
			str = gameLang->hstrings(HSTR_OFFICER_IN_POOL);
			break;

		case LeaderState::Working:
			location = ptr->location;

			if (ptr->eta) {
				str = gameLang->hstrings(HSTR_FLEET_OFFICER_ETA);
				buf.printf(str, ptr->eta);
				drawETA(x + img->width() / 2,
					y - 3 + img->height() / 2,
					FONT_COLOR_FLEETLIST_SPECDAMAGE,
					buf.c_str());
			}

			if (location < 0) {
				str = NULL;
			} else if (ptr->type == LEADER_TYPE_CAPTAIN) {
				str = _game->_ships[location].design.name;
			} else {
				str = _game->_starSystems[location].name;
			}

			break;

		case LeaderState::Unassigned:
			str = gameLang->hstrings(HSTR_OFFICER_UNASSIGNED);
			break;

		case LeaderState::ForHire:
			str = gameLang->hstrings(HSTR_OFFICER_FOR_HIRE_TIME);
			buf.printf(str, 30 - ptr->eta);
			str = buf.c_str();
			color2 = FONT_COLOR_FLEETLIST_SPECDAMAGE;
			break;

		default:
			str = NULL;
			break;
		}

		if (str) {
			smallfnt->centerText(x + img->width() / 2,
				y + img->height() + 5, color2, str,
				OUTLINE_FULL, 2);
		}

		// TODO: draw salary/hire price
	}

	redrawWidgets(0, 0, curtick);
	redrawWindows(curtick);
}

void LeaderListView::handleMouseMove(int x, int y, unsigned buttons) {
	if (_scrollgrab) {
		_scroll->handleMouseMove(x, y, buttons);
		return;
	}

	GuiView::handleMouseMove(x, y, buttons);
}

void LeaderListView::handleMouseUp(int x, int y, unsigned button) {
	if (_scrollgrab) {
		_scroll->handleMouseUp(x, y, button);
		return;
	}

	GuiView::handleMouseUp(x, y, button);
}

void LeaderListView::showHelp(int x, int y, int arg) {
	new MessageBoxWindow(this, arg, _bg->palette());
}

void LeaderListView::showSelectionHelp(int x, int y, int arg) {
	unsigned idx;

	if (_panelChoice->value()) {
		idx = HELP_LEADERLIST_SHIP_DESCRIPTION;
	} else {
		idx = HELP_LEADERLIST_STAR_DESCRIPTION;
	}

	new MessageBoxWindow(this, idx + arg, _bg->palette());
}

void LeaderListView::showSlotHelp(int x, int y, int arg) {
	int idx;

	if (_panelChoice->value()) {
		idx = HELP_LEADERLIST_CAPTAIN_LIST;
	} else {
		idx = HELP_LEADERLIST_ADMIN_LIST;
	}

	new MessageBoxWindow(this, idx, _bg->palette());
}

void LeaderListView::selectOfficerLocation(int x, int y, int arg) {
	unsigned i;
	int idx = getLeaderID(arg);
	const Leader *ptr;
	const char *str, *name = getRankedName(arg);
	StringBuffer buf;

	if (idx < 0) {
		return;
	}

	ptr = _game->_leaders + idx;

	if (ptr->status == LeaderState::Idle) {
		str = gameLang->hstrings(HSTR_OFFICER_LOCATION_POOL);
		buf.printf(str, name);
		new ErrorWindow(this, buf.c_str());
		return;
	} else if (ptr->status != LeaderState::Working || ptr->location < 0) {
		str = gameLang->hstrings(HSTR_OFFICER_LOCATION_NONE);
		buf.printf(str, name);
		new ErrorWindow(this, buf.c_str());
		return;
	}

	if (_panelChoice->value()) {
		const Ship *sptr = _game->_ships + ptr->location;

		if (sptr->status == ShipState::InRefit) {
			i = HSTR_OFFICER_LOCATION_REFIT;
			idx = sptr->getStarID();
			buf.printf(gameLang->hstrings(i), name,
				sptr->design.name,
				_game->_starSystems[idx].name);
			new ErrorWindow(this, buf.c_str());
			return;
		} else if (!sptr->isActive()) {
			i = HSTR_OFFICER_LOCATION_MISSING;
			buf.printf(gameLang->hstrings(i), name);
			new ErrorWindow(this, buf.c_str());
			return;
		}

		_minimap->selectFleet(_game->findFleet(sptr));
		fleetSelectionChanged(x, y, 0);
	} else {
		_minimap->selectStar(ptr->location);
		starSelectionChanged(x, y, 0);
	}
}

void LeaderListView::cancelSelect(int x, int y, int arg) {
	int oldsel = _selLeader;

	_selLeader = -1;
	slotStateChanged(oldsel);
	_grid->selectNone();
}

void LeaderListView::assignOfficer(int x, int y, int arg) {
	cancelSelect(0, 0, 0);
	new MessageBoxWindow(this, "Not implemented yet");
}

void LeaderListView::askAssignOfficer(void) {
	int location, oldloc = -1, eta = 0;
	const Leader *ptr;
	const char *locname = NULL;
	ConfirmationWindow *window;
	StringBuffer buf;

	if (_selLeader < 0) {
		cancelSelect(0, 0, 0);
		return;
	}

	ptr = _game->_leaders + getLeaderID(_selLeader);

	if (ptr->status == LeaderState::Working ||
		ptr->status == LeaderState::Unassigned) {
		oldloc = ptr->location;
	}

	if (_panelChoice->value()) {
		const Fleet *f1 = NULL, *f2 = _grid->getFleet();
		const Star *s1, *s2;

		location = _grid->selectedShipID();

		if (location >= 0) {
			locname = _game->_ships[location].design.name;
		}

		if (oldloc >= 0) {
			f1 = _game->findFleet(_game->_ships + oldloc);
		}

		s1 = f1 ? f1->getOrbitedStar() : NULL;
		s2 = f2 ? f2->getOrbitedStar() : NULL;

		// Reassignment is free only within the same fleet or
		// when two different fleets orbit the same star
		if (!f1 || (f1 != f2 && (!s1 || s1 != s2))) {
			eta = LEADER_MOVE_TIME;
		}
	} else {
		location = _minimap->selectedStar();

		if (location >= 0) {
			locname = _game->_starSystems[location].name;
		}

		// Reassignment is free only when the officer was just
		// unassigned from the same location
		if (ptr->status != LeaderState::Unassigned ||
			ptr->location != location) {
			eta = LEADER_MOVE_TIME;
		}
	}

	if (location < 0 || (ptr->status == LeaderState::Working &&
		ptr->location == location)) {
		cancelSelect(0, 0, 0);
		return;
	}

	if (eta) {
		buf.printf(gameLang->hstrings(HSTR_OFFICER_ASSIGN_SLOW), eta,
			getRankedName(_selLeader), locname);
	} else {
		buf.printf(gameLang->hstrings(HSTR_OFFICER_ASSIGN_INSTANT),
			getRankedName(_selLeader), locname);
	}

	window = new ConfirmationWindow(this, buf.c_str());
	window->setYesCallback(GuiMethod(*this,
		&LeaderListView::assignOfficer));
	window->setNoCallback(GuiMethod(*this, &LeaderListView::cancelSelect));
}

void LeaderListView::slotStateChanged(int slot) {
	unsigned color;

	if (slot < 0) {
		return;
	}

	if (slot == _curLeader || slot == _selLeader) {
		color = FONT_COLOR_LEADERLIST_BRIGHT;
	} else {
		color = FONT_COLOR_LEADERLIST_NORMAL;
	}

	_leaderSlots[slot]->setFontColor(color);
}

void LeaderListView::changePanel(int x, int y, int arg) {
	unsigned i, count, val = _panelChoice->value();
	unsigned *idlist;

	_curLeader = -1;
	_selLeader = -1;
	_starSystem->hide(val);
	_grid->hide(!val);
	_scroll->hide(!val);
	_scrollUp->hide(!val);
	_scrollDown->hide(!val);

	if (val) {
		idlist = _captains;
		count = _captainCount;
		_minimap->selectStar(-1);
		_minimap->selectFleet(_curFleet);
		fleetSelectionChanged(x, y, 0);
	} else {
		idlist = _admins;
		count = _adminCount;
		_minimap->selectFleet(NULL);
		_minimap->selectStar(_curStar);
		starSelectionChanged(x, y, 0);
	}

	for (i = 0; i < MAX_HIRED_LEADERS; i++) {
		slotStateChanged(i);
		_leaderSlots[i]->setLeader(i < count ? idlist[i] : -1);
	}
}

void LeaderListView::highlightSlot(int x, int y, int arg) {
	int oldsel = _curLeader;

	if (getLeaderID(arg) < 0) {
		arg = -1;
	}

	_curLeader = arg;
	slotStateChanged(oldsel);
	slotStateChanged(_curLeader);
}

void LeaderListView::selectSlot(int x, int y, int arg) {
	int idx, location, oldsel = _selLeader;
	Leader *ptr = NULL;

	idx = getLeaderID(arg);

	if (idx < 0) {
		return;
	}

	ptr = _game->_leaders + idx;

	if (ptr->status == LeaderState::ForHire) {
		// TODO: Ask whether to hire leader
		new MessageBoxWindow(this, "Not implemented yet");
		return;
	}

	if (_panelChoice->value()) {
		_selLeader = (_selLeader == arg) ? -1 : arg;
		slotStateChanged(oldsel);
		slotStateChanged(_selLeader);
		location = _grid->selectedShipID();

		if (location >= 0 && _selLeader >= 0) {
			askAssignOfficer();
		}

		return;
	} else {
		location = _minimap->selectedStar();

		if (ptr->status == LeaderState::Working &&
			ptr->location == location) {
			return;
		}

		_selLeader = (_selLeader == arg) ? -1 : arg;
		slotStateChanged(oldsel);
		slotStateChanged(_selLeader);
		askAssignOfficer();
		return;
	}
}

void LeaderListView::fleetHighlightChanged(int x, int y, int arg) {
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

void LeaderListView::fleetSelectionChanged(int x, int y, int arg) {
	Fleet *f;

	if (!_panelChoice->value()) {
		_minimap->selectFleet(NULL);
		return;
	}

	f = _minimap->selectedFleet();
	_curFleet = f;
	_grid->setFleet(f, -1);
	updateScrollbar();
	shipHighlightChanged(x, y, 0);
}

void LeaderListView::shipHighlightChanged(int x, int y, int arg) {
	const Ship *sptr = _grid->highlightedShip();

	if (!sptr) {
		_selectionLabel->clear();
		return;
	}

	if (sptr->officer >= 0) {
		StringBuffer buf;

		buf.printf("%s (%s)", sptr->design.name,
			_game->_leaders[sptr->officer].name);
		_selectionLabel->setText(buf.c_str(), FONTSIZE_MEDIUM,
			FONT_COLOR_LEADERLIST_NORMAL, OUTLINE_NONE,
			ALIGN_CENTER);
	} else {
		_selectionLabel->setText(sptr->design.name, FONTSIZE_MEDIUM,
			FONT_COLOR_LEADERLIST_NORMAL, OUTLINE_NONE,
			ALIGN_CENTER);
	}
}

void LeaderListView::shipSelectionChanged(int x, int y, int arg) {
	const Ship *sptr = _grid->selectedShip();
	const char *str;

	if (sptr && sptr->design.type != ShipType::COMBAT_SHIP) {
		_grid->selectNone();
		str =  gameLang->hstrings(HSTR_OFFICER_ASSIGN_NONCOMBAT);
		new ErrorWindow(this, str);
		return;
	}

	if (_selLeader >= 0 && sptr) {
		askAssignOfficer();
	}
}

void LeaderListView::starHighlightChanged(int x, int y, int arg) {
	unsigned color;
	int idx, star = _minimap->highlightedStar();
	const Star *sptr;

	if (star < 0) {
		_minimapLabel->clear();
		return;
	}

	sptr = _game->_starSystems + star;

	if (sptr->owner < 0) {
		color = MAX_PLAYERS;
	} else {
		color = _game->_players[sptr->owner].color;
	}

	color = leaderListMinimapStarColors[color];
	idx = sptr->officerIndex[_activePlayer];

	if (idx < 0) {
		_minimapLabel->setText(sptr->name, FONTSIZE_MEDIUM, color,
			OUTLINE_NONE, ALIGN_CENTER);
	} else {
		StringBuffer buf;

		buf.printf("%s (%s)", sptr->name, _game->_leaders[idx].name);
		_minimapLabel->setText(buf.c_str(), FONTSIZE_MEDIUM, color,
			OUTLINE_NONE, ALIGN_CENTER);
	}
}

void LeaderListView::starSelectionChanged(int x, int y, int arg) {
	int idx, star = _minimap->selectedStar();

	if (_panelChoice->value()) {
		_minimap->selectStar(-1);
		return;
	}

	if (star < 0) {
		return;
	}

	_curStar = star;
	_starSystem->setStar(star);
	idx = _game->_starSystems[star].officerIndex[_activePlayer];

	if (idx >= 0) {
		StringBuffer buf;

		buf.printf("%s (%s)", _game->_starSystems[star].name,
			_game->_leaders[idx].name);
		_selectionLabel->setText(buf.c_str(), FONTSIZE_MEDIUM,
			FONT_COLOR_LEADERLIST_NORMAL, OUTLINE_NONE,
			ALIGN_CENTER);
	} else {
		_selectionLabel->setText(_game->_starSystems[star].name,
			FONTSIZE_MEDIUM, FONT_COLOR_LEADERLIST_NORMAL,
			OUTLINE_NONE, ALIGN_CENTER);
	}
}

void LeaderListView::handleBeginScroll(int x, int y, int arg) {
	_scrollgrab = 1;
}

void LeaderListView::handleScroll(int x, int y, int arg) {
	_grid->setScroll(_scroll->position());
}

void LeaderListView::handleEndScroll(int x, int y, int arg) {
	_scrollgrab = 0;
	selectCurrentWidget(x, y, 0);
}

void LeaderListView::updateScrollbar(void) {
	unsigned cols = _grid->cols();

	_scroll->setRange((_grid->visibleShipCount() + cols - 1) / cols);
}

void LeaderListView::clickSelectPrevious(int x, int y, int arg) STUB(this)

void LeaderListView::clickSelectNext(int x, int y, int arg) STUB(this)

void LeaderListView::clickHireButton(int x, int y, int arg) STUB(this)

void LeaderListView::clickPoolButton(int x, int y, int arg) STUB(this)

void LeaderListView::clickDismissButton(int x, int y, int arg) STUB(this)

void LeaderListView::clickReturn(int x, int y, int arg) {
	exitView();
}
