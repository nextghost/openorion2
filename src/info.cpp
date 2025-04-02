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

#include <cstring>
#include "lang.h"
#include "screen.h"
#include "info.h"

#define INFO_ARCHIVE "info.lbx"
#define ASSET_INFO_BG 0
#define ASSET_INFO_CURSOR 1
#define ASSET_INFO_RETURN_BUTTON 2
#define ASSET_INFO_HISTORY_BUTTON 3
#define ASSET_INFO_VBAR 22
#define ASSET_INFO_VBAR_MASK 23
#define ASSET_INFO_BAR_LABEL 24
#define ASSET_INFO_VBAR_FOOT 25

#define INFO_BAR_COUNT 7
#define INFO_BAR_COLORS 8

static const uint8_t bar_color_maps[INFO_BAR_COUNT][INFO_BAR_COLORS] = {
	{0, 176, 152, 153, 154, 155, 156, 157},
	{0, 176, 13, 15, 17, 19, 20, 22},
	{0, 176, 164, 165, 166, 167, 168, 169},
	{0, 176, 170, 171, 172, 173, 174, 175},
	{0, 176, 158, 159, 160, 161, 162, 163},
	{0, 176, 146, 147, 148, 149, 150, 151},
	{0, 176, 181, 182, 183, 184, 185, 186},
};

static void drawInfoBox(int x, int y, unsigned width, unsigned height) {
	unsigned ypos;

	if (width < 7 || height < 7) {
		return;
	}

	gameScreen->fillRect(x + 2, y, 1, 1, RGB(0x084408));
	gameScreen->fillRect(x + 3, y, width - 6, 1, RGB(0x083808));
	gameScreen->fillRect(x + width - 3, y, 1, 1, RGB(0x082808));
	gameScreen->fillRect(x + 2, y + 1, 1, 2, RGB(0x082808));
	gameScreen->fillRect(x + 3, y + 1, width - 6, 2, RGB(0x081c08));
	gameScreen->fillRect(x + width - 3, y + 1, 1, 2, RGB(0x080c08));

	ypos = y + 3;
	gameScreen->fillRect(x, ypos, 1, height - 6, RGB(0x082808));
	gameScreen->fillRect(x + 1, ypos, width - 2, height - 6, RGB(0x081c08));
	gameScreen->fillRect(x + width - 1, ypos, 1, height - 6, RGB(0x080c08));

	for (; ypos < y + height - 3; ypos += 3) {
		gameScreen->fillRect(x, ypos, 1, 1, RGB(0x084408));
		gameScreen->fillRect(x + 1, ypos, width - 2, 1, RGB(0x083808));
		gameScreen->fillRect(x + width - 1, ypos, 1, 1, RGB(0x082808));
	}

	gameScreen->fillRect(x + 2, y + height - 3, 1, 1, RGB(0x084408));
	gameScreen->fillRect(x + 3, y + height - 3, width - 6, 1,
		RGB(0x083808));
	gameScreen->fillRect(x + width - 3, y + height - 3, 1, 1,
		RGB(0x082808));
	gameScreen->fillRect(x + 2, y + height - 2, 1, 2, RGB(0x082808));
	gameScreen->fillRect(x + 3, y + height - 2, width - 6, 2,
		RGB(0x081c08));
	gameScreen->fillRect(x + width - 3, y + height - 2, 1, 2,
		RGB(0x080c08));
}

HistoryGraphWidget::HistoryGraphWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, const GameState *game, int activePlayer) :
	Widget(x, y, width, height), _game(game), _activePlayer(activePlayer) {

}

HistoryGraphWidget::~HistoryGraphWidget(void) {

}

void HistoryGraphWidget::redraw(int x, int y, unsigned curtick) {
	Font *titleFnt;
	const char *str;

	if (isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	titleFnt = gameFonts->getFont(FONTSIZE_TITLE);

	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_TITLE_HISTORY);
	titleFnt->centerText(x + 208, y + 31, TITLE_COLOR_INFO, str,
		OUTLINE_NONE, 3);

	drawInfoBox(x + 14, y + 60, 390, 63);
	drawInfoBox(x + 14, y + 132, 390, 282);
}

TechReviewWidget::TechReviewWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, const GameState *game, int activePlayer) :
	Widget(x, y, width, height), _game(game), _activePlayer(activePlayer) {

}

TechReviewWidget::~TechReviewWidget(void) {

}

void TechReviewWidget::redraw(int x, int y, unsigned curtick) {
	Font *titleFnt;
	const char *str;

	if (isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	titleFnt = gameFonts->getFont(FONTSIZE_TITLE);

	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_TITLE_TECH);
	titleFnt->centerText(x + 208, y + 31, TITLE_COLOR_INFO, str,
		OUTLINE_NONE, 3);

	drawInfoBox(x + 12, y + 60, 208, 354);
	drawInfoBox(x + 227, y + 60, 180, 45);
	drawInfoBox(x + 227, y + 264, 180, 150);
}

RaceInfoWidget::RaceInfoWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, const GameState *game, int activePlayer) :
	Widget(x, y, width, height), _game(game), _activePlayer(activePlayer),
	_page(0) {

}

RaceInfoWidget::~RaceInfoWidget(void) {

}

void RaceInfoWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, j, pcount, color;
	int xpos, ypos;
	Font *titleFnt, *raceFnt, *itemFnt;
	const char *str;
	const Player *pptr, *players[MAX_PLAYERS] = { NULL };
	StringBuffer buf;

	if (isHidden()) {
		return;
	}

	players[0] = _game->_players + _activePlayer;

	for (i = 0, pcount = 1; i < _game->_playerCount; i++) {
		if (_activePlayer == (int)i || _game->_players[i].eliminated ||
			!players[0]->isPlayerVisible(i)) {
			continue;
		}

		players[pcount++] = _game->_players + i;
	}

	// Eliminated players are listed last regardless of visibility
	for (i = 0; i < _game->_playerCount; i++) {
		if (_game->_players[i].eliminated) {
			players[pcount++] = _game->_players + i;
		}
	}

	x += getX();
	y += getY();
	titleFnt = gameFonts->getFont(FONTSIZE_TITLE);
	raceFnt = gameFonts->getFont(FONTSIZE_BIG);
	itemFnt = gameFonts->getFont(FONTSIZE_SMALL);

	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_TITLE_RACE);
	titleFnt->centerText(x + 208, y + 31, TITLE_COLOR_INFO, str,
		OUTLINE_NONE, 3);

	for (i = 0; i < 4; i++) {
		xpos = x + (i % 2 ? 213 : 17);
		ypos = y + (i / 2 ? 243 : 60);
		drawInfoBox(xpos, ypos, 188, 171);
		gameScreen->fillRect(xpos + 13, ypos + 21, 164, 1,
			RGB(0x0c840c));

		pptr = players[i + 4 * _page];

		if (!pptr) {
			str = gameLang->misctext(TXT_MISC_BILLTEXT,
				BILL_INFO_NO_CONTACT);
			raceFnt->centerText(xpos + 93, ypos + 4,
				FONT_COLOR_INFO_NORMAL, str, OUTLINE_NONE, 2);
			continue;
		}

		buf = pptr->race;
		buf.toUpper();
		color = FONT_COLOR_INFO_RED + pptr->color;
		raceFnt->centerText(xpos + 93, ypos + 4, color, buf.c_str(),
			OUTLINE_NONE, 2);

		if (pptr->eliminated) {
			str = gameLang->misctext(TXT_MISC_BILLTEXT,
				BILL_INFO_ELIMINATED);
			itemFnt->centerText(xpos + 93, ypos + 17,
				FONT_COLOR_INFO_ELIMINATED, str, OUTLINE_FULL,
				2);
		}

		ypos += 26;
		str = gameLang->estrings(ESTR_GOVERNMENT_NAMES +
			pptr->traits[TRAIT_GOVERNMENT]);
		buf.printf("^ %s", str);
		itemFnt->renderText(xpos + 3, ypos, color, buf.c_str(),
			OUTLINE_NONE, 2);
		ypos += itemFnt->height() + 1;

		for (j = 1; j < TRAITS_COUNT; j++) {
			if (!pptr->traits[j] || (j == TRAIT_RICH_HOMEWORLD &&
				pptr->traits[j] < 0)) {
				continue;
			}

			str = gameLang->raceInfo(j);

			if (j < TRAIT_LOW_G) {
				buf.printf("^ %s%+d", str, pptr->traits[j]);
			} else {
				buf.printf("^ %s", str);
			}

			itemFnt->renderText(xpos + 3, ypos, color, buf.c_str(),
				OUTLINE_NONE, 2);
			ypos += itemFnt->height() + 1;
		}

		if (pptr->traits[TRAIT_RICH_HOMEWORLD] < 0) {
			str = gameLang->raceInfo(TRAIT_POOR_HOMEWORLD);
			buf.printf("^ %s", str);
			itemFnt->renderText(xpos + 3, ypos, color, buf.c_str(),
				OUTLINE_NONE, 2);
		}
	}
}

TurnSummaryWidget::TurnSummaryWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, const GameState *game, int activePlayer) :
	Widget(x, y, width, height), _game(game), _activePlayer(activePlayer) {

}

TurnSummaryWidget::~TurnSummaryWidget(void) {

}

void TurnSummaryWidget::redraw(int x, int y, unsigned curtick) {
	Font *titleFnt;
	const char *str;

	if (isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	titleFnt = gameFonts->getFont(FONTSIZE_TITLE);

	str = gameLang->misctext(TXT_MISC_BILLTEXT,
		BILL_INFO_TITLE_TURN_SUMMARY);
	titleFnt->centerText(x + 208, y + 31, TITLE_COLOR_INFO, str,
		OUTLINE_NONE, 3);

	drawInfoBox(x + 12, y + 60, 395, 355);
}

DocsWidget::DocsWidget(unsigned x, unsigned y, unsigned width,
	unsigned height) : Widget(x, y, width, height) {

}

DocsWidget::~DocsWidget(void) {

}

void DocsWidget::redraw(int x, int y, unsigned curtick) {
	Font *titleFnt;
	const char *str;

	if (isHidden()) {
		return;
	}

	x += getX();
	y += getY();
	titleFnt = gameFonts->getFont(FONTSIZE_TITLE);

	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_TITLE_REFERENCE);
	titleFnt->centerText(x + 208, y + 31, TITLE_COLOR_INFO, str,
		OUTLINE_NONE, 3);

	drawInfoBox(x + 12, y + 60, 197, 27);
	drawInfoBox(x + 12, y + 93, 197, 321);
	drawInfoBox(x + 212, y + 60, 196, 27);
	drawInfoBox(x + 212, y + 93, 196, 321);
}

InfoView::InfoView(GameState *game, int activePlayer) : _game(game),
	_activePlayer(activePlayer), _panelChoice(NULL) {

	ImageAsset cursor;
	const uint8_t *pal;

	cursor = gameAssets->getImage(INFO_ARCHIVE, ASSET_INFO_CURSOR);
	pal = cursor->palette();
	_bg = gameAssets->getImage(INFO_ARCHIVE, ASSET_INFO_BG, pal);
	_vbar = gameAssets->getBitmap(INFO_ARCHIVE, ASSET_INFO_VBAR);
	_barMask = gameAssets->getBitmap(INFO_ARCHIVE, ASSET_INFO_VBAR_MASK);
	_barLabel = gameAssets->getBitmap(INFO_ARCHIVE, ASSET_INFO_BAR_LABEL);
	_barFoot = gameAssets->getBitmap(INFO_ARCHIVE, ASSET_INFO_VBAR_FOOT);

	initWidgets();
}

InfoView::~InfoView(void) {

}

void InfoView::initWidgets(void) {
	unsigned i, y;
	ImageAsset asset;
	Widget *w;
	const uint8_t *pal = _bg->palette();

	w = createWidget(535, 434, INFO_ARCHIVE, ASSET_INFO_RETURN_BUTTON,
		pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod<InfoView>(*this,
		&InfoView::clickReturn));

	_panelChoice = new ChoiceWidget(21, 50, 164, 131, INFO_PANEL_COUNT);
	addWidget(_panelChoice);

	for (i = 0, y = 0; i < INFO_PANEL_COUNT; i++) {
		asset = gameAssets->getImage(INFO_ARCHIVE,
			ASSET_INFO_HISTORY_BUTTON + i, pal);
		_panelChoice->setChoiceButton(i, 0, y, 164, asset->height(),
			(Image*)asset, 1);
		_panelChoice->button(i)->setIdleSprite((Image*)asset, 0);
		y += asset->height();
	}

	_panelChoice->setValueChangeCallback(GuiMethod(*this,
		&InfoView::changePanel));
	i = _game->_players[_activePlayer].infoPanel >> 4;
	_panelChoice->setValue(i < INFO_PANEL_COUNT ? i : 0);

	_panels[0] = new HistoryGraphWidget(206, 0, SCREEN_WIDTH - 206,
		SCREEN_HEIGHT, _game, _activePlayer);
	addWidget(_panels[0]);
	_panels[1] = new TechReviewWidget(206, 0, SCREEN_WIDTH - 206,
		SCREEN_HEIGHT, _game, _activePlayer);
	addWidget(_panels[1]);
	_panels[2] = new RaceInfoWidget(206, 0, SCREEN_WIDTH - 206,
		SCREEN_HEIGHT, _game, _activePlayer);
	addWidget(_panels[2]);
	_panels[3] = new TurnSummaryWidget(206, 0, SCREEN_WIDTH - 206,
		SCREEN_HEIGHT, _game, _activePlayer);
	addWidget(_panels[3]);
	_panels[4] = new DocsWidget(206, 0, SCREEN_WIDTH - 206, SCREEN_HEIGHT);
	addWidget(_panels[4]);

	changePanel(0, 0, 0);
}

void InfoView::changePanel(int x, int y, int arg) {
	unsigned i;

	for (i = 0; i < INFO_PANEL_COUNT; i++) {
		_panels[i]->hide(i != _panelChoice->value());
	}

	// TODO: Change Player::infoPanel in gamestate
}

void InfoView::redraw(unsigned curtick) {
	unsigned i, x, y, count, date, maxcost = 0, maintcosts[INFO_BAR_COUNT];
	Font *dateFnt, *smallFnt, *labelFnt;
	const Player *pptr = _game->_players + _activePlayer;
	const char *str;
	const uint8_t *pal = _bg->palette();
	StringBuffer buf;
	uint8_t barpal[PALSIZE];

	maintcosts[0] = pptr->bcProduced;
	maintcosts[1] = pptr->buildingMaintenance;
	maintcosts[2] = pptr->freighterMaintenance;
	maintcosts[3] = pptr->shipMaintenance;
	maintcosts[4] = pptr->spyMaintenance;
	maintcosts[5] = pptr->tributeCost;
	maintcosts[6] = pptr->officerMaintenance;
	memcpy(barpal, pal, PALSIZE * sizeof(uint8_t));

	for (i = 0; i < INFO_BAR_COUNT; i++) {
		maxcost = MAX(maxcost, maintcosts[i]);
	}

	for (i = 0; i < INFO_BAR_COUNT; i++) {
		maintcosts[i] = (100 * maintcosts[i] + maxcost / 2) / maxcost;
	}

	dateFnt = gameFonts->getFont(FONTSIZE_BIG);
	smallFnt = gameFonts->getFont(FONTSIZE_SMALLER);
	labelFnt = gameFonts->getFont(FONTSIZE_SMALL);

	gameScreen->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGB(0x000000));
	gameScreen->fillRect(212, 23, 408, 434, RGB(0x083808));

	for (y = 23; y < 457; y += 3) {
		gameScreen->fillRect(212, y, 408, 1, RGB(0x082808));
	}

	_bg->draw(0, 0);
	date = _game->_gameConfig.stardate;
	buf.printf("%u.%u", date / 10, date % 10);
	dateFnt->centerText(150, 27, FONT_COLOR_INFO_NORMAL, buf.c_str(),
		OUTLINE_NONE, 2);

	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_INCOME_BAR);
	smallFnt->renderText(27, 324, FONT_COLOR_INFO_NORMAL, str);
	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_MAINTENANCE);
	smallFnt->renderText(91, 324, FONT_COLOR_INFO_NORMAL, str);

	y = 344;
	remapColors(barpal, pal, bar_color_maps[0], INFO_BAR_COLORS);
	_barLabel->draw(25, y, barpal);
	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_INFO_INCOME_LABEL);
	buf.printf("%u%s", pptr->bcProduced, str);
	labelFnt->centerText(116, y + 3, FONT_COLOR_INFO_BLACK, buf.c_str(),
		OUTLINE_NONE, 2);
	y += _barLabel->height() - 2;

	for (i = 1, count = 0; i < INFO_BAR_COUNT; i++) {
		remapColors(barpal, pal, bar_color_maps[i], INFO_BAR_COLORS);
		_barLabel->draw(25, y, barpal);
		buf.printf("%u%%", maintcosts[i]);
		labelFnt->centerText(65, y + 3, FONT_COLOR_INFO_BLACK,
			buf.c_str(), OUTLINE_NONE, 2);
		str = gameLang->misctext(TXT_MISC_BILLTEXT,
			BILL_INFO_INCOME_LABEL + i);
		labelFnt->centerText(132, y + 3, FONT_COLOR_INFO_BLACK, str,
			OUTLINE_NONE, 2);
		y += _barLabel->height() - 2;

		if (maintcosts[i]) {
			count++;
		}
	}

	remapColors(barpal, pal, bar_color_maps[0], INFO_BAR_COLORS);
	_vbar->drawTile(30, 212 + 100 - maintcosts[0], 0, 0, _vbar->width(),
		maintcosts[0], barpal);
	_vbar->drawTileMasked(30, 311, 0, maintcosts[0] - 1, _vbar->width(),
		_barMask->height(), barpal, (const Bitmap*)_barMask);
	_barFoot->draw(30, 314, barpal);
	x = 123 - _vbar->width() + (count * (_vbar->width() - 6)) / 2;

	for (i = INFO_BAR_COUNT - 1; i; i--) {
		if (!maintcosts[i]) {
			continue;
		}

		remapColors(barpal, pal, bar_color_maps[i], INFO_BAR_COLORS);
		_vbar->drawTile(x, 212 + 100 - maintcosts[i], 0, 0,
			_vbar->width(), maintcosts[i], barpal);
		_vbar->drawTileMasked(x, 311, 0, maintcosts[i] - 1,
			_vbar->width(), _barMask->height(), barpal,
			(const Bitmap*)_barMask);
		_barFoot->draw(x, 314, barpal);
		x -= _vbar->width() - 6;
	}

	redrawWidgets(0, 0, curtick);
}

void InfoView::clickReturn(int x, int y, int arg) {
	exitView();
}
