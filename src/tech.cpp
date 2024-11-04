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
#include "galaxy.h"
#include "tech.h"

#define SCIENCE_ARCHIVE "science.lbx"
#define ASSET_SCIENCE_CURSOR 0

#define TECHSEL_ARCHIVE "techsel.lbx"
#define ASSET_TECHSEL_BG 0
#define ASSET_TECHSEL_AREA_BUTTONS 1
#define ASSET_TECHSEL_CANCEL_BUTTON 27
#define ASSET_TECHLIST_BG 23
#define ASSET_TECHLIST_RETURN_BUTTON 24
#define ASSET_TECHLIST_UP_BUTTON 25
#define ASSET_TECHLIST_DOWN_BUTTON 26
#define TECHSEL_CANCEL_OFFSET 14

#define MAX_AREA_TOPICS 14

#define TECH_HIGHLIGHT_FRAMECOUNT 6
#define TECH_HIGHLIGHT_SPEED 80

#define TECHLIST_TITLE_SPACING 4
#define TECHLIST_ITEM_SPACING 2
#define TECHLIST_GROUP_SPACING 8

static const uint8_t techHighlightColors[TECH_HIGHLIGHT_FRAMECOUNT * 3] = {
	RGB(0x248000), RGB(0x3c9804), RGB(0x44a008), RGB(0x5cb80c),
	RGB(0x7cd814), RGB(0x9cf81c)
};

static const ResearchArea area_list[MAX_RESEARCH_AREAS] = {
	RESEARCH_CONSTRUCTION, RESEARCH_POWER, RESEARCH_CHEMISTRY,
	RESEARCH_SOCIOLOGY, RESEARCH_COMPUTERS, RESEARCH_BIOLOGY,
	RESEARCH_PHYSICS, RESEARCH_FIELDS
};

static const ResearchTopic techtree[MAX_RESEARCH_AREAS][MAX_AREA_TOPICS] = {
	{ // Biology
		TOPIC_ASTRO_BIOLOGY,
		TOPIC_ADVANCED_BIOLOGY,
		TOPIC_GENETIC_ENGINEERING,
		TOPIC_GENETIC_MUTATIONS,
		TOPIC_MACRO_GENETICS,
		TOPIC_EVOLUTIONARY_GENETICS,
		TOPIC_ARTIFICIAL_LIFE,
		TOPIC_TRANS_GENETICS,
		TOPIC_HYPER_BIOLOGY,
	},
	{ // Power
		TOPIC_NUCLEAR_FISSION,
		TOPIC_COLD_FUSION,
		TOPIC_ADVANCED_FUSION,
		TOPIC_ION_FISSION,
		TOPIC_ANTIMATTER_FISSION,
		TOPIC_MATTER_ENERGY_CONVERSION,
		TOPIC_HIGH_ENERGY_DISTRIBUTION,
		TOPIC_HYPER_DIMENSIONAL_FISSION,
		TOPIC_INTERPHASED_FISSION,
		TOPIC_HYPER_POWER,
	},
	{ // Physics
		TOPIC_PHYSICS,
		TOPIC_FUSION_PHYSICS,
		TOPIC_TACHYON_PHYSICS,
		TOPIC_NEUTRINO_PHYSICS,
		TOPIC_ARTIFICIAL_GRAVITY,
		TOPIC_SUBSPACE_PHYSICS,
		TOPIC_MULTIPHASED_PHYSICS,
		TOPIC_PLASMA_PHYSICS,
		TOPIC_MULTIDIMENSIONAL_PHYSICS,
		TOPIC_HYPER_DIMENSIONAL_PHYSICS,
		TOPIC_TEMPORAL_PHYSICS,
		TOPIC_HYPER_PHYSICS,
	},
	{ // Construction
		TOPIC_ENGINEERING,
		TOPIC_ADVANCED_ENGINEERING,
		TOPIC_ADVANCED_CONSTRUCTION,
		TOPIC_CAPSULE_CONSTRUCTION,
		TOPIC_ASTRO_ENGINEERING,
		TOPIC_ROBOTICS,
		TOPIC_SERVO_MECHANICS,
		TOPIC_ASTRO_CONSTRUCTION,
		TOPIC_ADVANCED_MANUFACTURING,
		TOPIC_ADVANCED_ROBOTICS,
		TOPIC_TECTONIC_ENGINEERING,
		TOPIC_SUPERSCALAR_CONSTRUCTION,
		TOPIC_PLANETOID_CONSTRUCTION,
		TOPIC_HYPER_CONSTRUCTION,
	},
	{ // Force Fields
		TOPIC_ADVANCED_MAGNETISM,
		TOPIC_GRAVITIC_FIELDS,
		TOPIC_MAGNETO_GRAVITICS,
		TOPIC_ELECTROMAGNETIC_REFRACTION,
		TOPIC_WARP_FIELDS,
		TOPIC_SUBSPACE_FIELDS,
		TOPIC_DISTORTION_FIELDS,
		TOPIC_QUANTUM_FIELDS,
		TOPIC_TRANSWARP_FIELDS,
		TOPIC_TEMPORAL_FIELDS,
		TOPIC_HYPER_FIELDS,
	},
	{ // Chemistry
		TOPIC_CHEMISTRY,
		TOPIC_ADVANCED_METALLURGY,
		TOPIC_ADVANCED_CHEMISTRY,
		TOPIC_MOLECULAR_COMPRESSION,
		TOPIC_NANO_TECHNOLOGY,
		TOPIC_MOLECULAR_MANIPULATION,
		TOPIC_MOLECULAR_CONTROL,
		TOPIC_HYPER_CHEMISTRY,
	},
	{ // Computers
		TOPIC_ELECTRONICS,
		TOPIC_OPTRONICS,
		TOPIC_ARTIFICIAL_INTELLIGENCE,
		TOPIC_POSITRONICS,
		TOPIC_ARTIFICIAL_CONSCIOUSNESS,
		TOPIC_CYBERTRONICS,
		TOPIC_CYBERTECHNICS,
		TOPIC_GALACTIC_NETWORKING,
		TOPIC_MOLECULATRONICS,
		TOPIC_HYPER_COMPUTERS,
	},
	{ // Sociology
		TOPIC_MILITARY_TACTICS,
		TOPIC_XENO_RELATIONS,
		TOPIC_MACRO_ECONOMICS,
		TOPIC_TEACHING_METHODS,
		TOPIC_ADVANCED_GOVERNMENTS,
		TOPIC_GALACTIC_ECONOMICS,
		TOPIC_HYPER_SOCIOLOGY,
	},
};

const ResearchChoice research_choices[MAX_RESEARCH_TOPICS] = {
	// TOPIC_STARTING_TECH, already researched at game start
	{0},
	{400, 0, {TECH_CLONING_CENTER, TECH_DEATH_SPORES,
		TECH_SOIL_ENRICHMENT}},
	{650, 0, {TECH_MERCULITE_MISSILE, TECH_POLLUTION_PROCESSOR}},
	{150, 0, {TECH_AUTOMATED_FACTORIES, TECH_HEAVY_ARMOR,
		TECH_PLANETARY_MISSILE_BASE}},
	{80, 0, {TECH_ANTIMISSILE_ROCKETS, TECH_REINFORCED_HULL,
		TECH_FIGHTER_BAYS}},
	{250, 0, {TECH_AUGMENTED_ENGINES, TECH_FUSION_BOMB, TECH_FUSION_DRIVE}},
	{4500, 0, {TECH_CONFEDERATION, TECH_FEDERATION,
		TECH_GALACTIC_UNIFICATION, TECH_IMPERIUM}},
	{250, 0, {TECH_CLASS_I_SHIELD, TECH_ECM_JAMMER, TECH_MASS_DRIVER}},
	{1500, 0, {TECH_PLANET_CONSTRUCTION, TECH_AUTOMATED_REPAIR_UNIT,
		TECH_RECYCLOTRON}},
	{250, 0, {TECH_DEUTERIUM_FUEL_CELLS, TECH_TRITANIUM_ARMOR}},

	// TOPIC_MILITARY_TACTICS
	{150, 0, {TECH_SPACE_ACADEMY}},
	{2000, 0, {TECH_BOMBER_BAYS, TECH_ROBOTIC_FACTORY}},
	{2000, 0, {TECH_ASTRO_UNIVERSITY}},
	{2000, 0, {TECH_ANTIMATTER_BOMB, TECH_ANTIMATTER_DRIVE,
		TECH_ANTIMATTER_TORPEDOES}},
	{1500, 0, {TECH_CYBERSECURITY_LINK, TECH_EMISSIONS_GUIDANCE_SYSTEM,
		TECH_RANGEMASTER_UNIT}},
	{400, 0, {TECH_NEURAL_SCANNER, TECH_SCOUT_LAB, TECH_SECURITY_STATIONS}},
	{1150, 0, {TECH_GRAVITON_BEAM, TECH_PLANETARY_GRAVITY_GENERATOR,
		TECH_TRACTOR_BEAM}},
	{4500, 0, {TECH_BIOTERMINATOR, TECH_UNIVERSAL_ANTIDOTE}},
	{80, 0, {TECH_BIOSPHERES, TECH_HYDROPONIC_FARM}},
	{1150, 0, {TECH_BATTLEOIDS, TECH_GROUND_BATTERIES,
		TECH_TITAN_CONSTRUCTION}},

	// TOPIC_ASTRO_ENGINEERING
	{400, 0, {TECH_ARMOR_BARRACKS, TECH_FIGHTER_GARRISON, TECH_SPACEPORT}},
	{250, 0, {TECH_BATTLE_PODS, TECH_SURVIVAL_PODS, TECH_TROOP_PODS}},
	{50, 1, {TECH_EXTENDED_FUEL_TANKS, TECH_NUCLEAR_MISSILE,
		TECH_STANDARD_FUEL_CELLS, TECH_TITANIUM_ARMOR}},
	{80, 0, {TECH_COLONY_SHIP, TECH_OUTPOST_SHIP, TECH_TRANSPORT}},
	{3500, 0, {TECH_ANDROID_FARMERS, TECH_ANDROID_SCIENTISTS,
		TECH_ANDROID_WORKERS}},
	{2750, 0, {TECH_AUTOLAB, TECH_CYBERTRONIC_COMPUTER,
		TECH_STRUCTURAL_ANALYZER}},
	{3500, 0, {TECH_CLOAKING_DEVICE, TECH_HARD_SHIELDS, TECH_STASIS_FIELD}},
	{1500, 0, {TECH_PERSONAL_SHIELD, TECH_STEALTH_FIELD,
		TECH_STEALTH_SUIT}},
	{50, 1, {TECH_ELECTRONIC_COMPUTER}},

	// TOPIC_ENGINEERING, already researched at game start
	{50, 1, {TECH_COLONY_BASE, TECH_MARINE_BARRACKS, TECH_STAR_BASE}},
	{2750, 0, {TECH_HEIGHTENED_INTELLIGENCE, TECH_PSIONICS}},
	{150, 0, {TECH_FUSION_BEAM, TECH_FUSION_RIFLE}},
	{6000, 0, {TECH_GALACTIC_CURRENCY_EXCHANGE}},
	{4500, 0, {TECH_GALACTIC_CYBERNET, TECH_VIRTUAL_REALITY_NETWORK}},
	{900, 0, {TECH_MICROBIOTICS, TECH_TELEPATHIC_TRAINING}},
	{1150, 0, {TECH_TERRAFORMING}},
	{650, 0, {TECH_ANTIGRAV_HARNESS, TECH_GYRO_DESTABILIZER,
		TECH_INERTIAL_STABILIZER}},
	{3500, 0, {TECH_ENERGY_ABSORBER, TECH_HIGH_ENERGY_FOCUS,
		TECH_MEGAFLUXERS}},
	{4500, 0, {TECH_HYPER_DRIVE, TECH_HYPERX_CAPACITORS,
		TECH_PROTON_TORPEDOES}},
	{6000, 0, {TECH_HYPERSPACE_COMMUNICATIONS, TECH_MAULER_DEVICE,
		TECH_SENSORS}},

	// TOPIC_INTERPHASED_FISSION
	{10000, 0, {TECH_INTERPHASED_DRIVE, TECH_NEUTRONIUM_BOMB,
		TECH_PLASMA_TORPEDOES}},
	{900, 0, {TECH_ION_DRIVE, TECH_ION_PULSE_CANNON,
		TECH_SHIELD_CAPACITORS}},
	{6000, 0, {TECH_ADVANCED_CITY_PLANNING, TECH_HEAVY_FIGHTER_BAYS,
		TECH_STAR_FORTRESS}},
	{1150, 0, {TECH_PLANETARY_STOCK_EXCHANGE}},
	{1500, 0, {TECH_SUBTERRANEAN_FARMS, TECH_WEATHER_CONTROL_SYSTEM}},
	{900, 0, {TECH_CLASS_III_SHIELD, TECH_PLANETARY_RADIATION_SHIELD,
		TECH_WARP_DISSIPATER}},
	{2750, 0, {TECH_FOOD_REPLICATORS, TECH_TRANSPORTERS}},
	{1150, 0, {TECH_ATMOSPHERIC_RENEWER, TECH_IRIDIUM_FUEL_CELLS,
		TECH_PULSON_MISSILE}},
	{10000, 0, {TECH_ADAMANTIUM_ARMOR, TECH_THORIUM_FUEL_CELLS}},
	{6000, 0, {TECH_ACHILLES_TARGETING_UNIT, TECH_MOLECULARTRONIC_COMPUTER,
		TECH_PLEASURE_DOME}},

	// TOPIC_MOLECULAR_MANIPULATION
	{4500, 0, {TECH_NEUTRONIUM_ARMOR, TECH_URRIDIUM_FUEL_CELLS,
		TECH_ZEON_MISSILE}},
	{4500, 0, {TECH_DIMENSIONAL_PORTAL, TECH_DISRUPTER_CANNON}},
	{2000, 0, {TECH_MULTIPHASED_SHIELDS, TECH_PHASOR, TECH_PHASOR_RIFLE}},
	{2000, 0, {TECH_MICROLITE_CONSTRUCTION, TECH_NANO_DISASSEMBLERS,
		TECH_ZORTRIUM_ARMOR}},
	{900, 0, {TECH_NEUTRON_BLASTER, TECH_NEUTRON_SCANNER}},
	{50, 1, {TECH_FREIGHTERS, TECH_NUCLEAR_BOMB, TECH_NUCLEAR_DRIVE}},
	{150, 0, {TECH_DAUNTLESS_GUIDANCE_SYSTEM, TECH_OPTRONIC_COMPUTER,
		TECH_RESEARCH_LABORATORY}},
	{50, 1, {TECH_LASER_CANNON, TECH_LASER_RIFLE, TECH_SPACE_SCANNER}},
	{7500, 0, {TECH_ARTEMIS_SYSTEM_NET, TECH_DOOM_STAR_CONSTRUCTION}},
	{3500, 0, {TECH_PLASMA_CANNON, TECH_PLASMA_RIFLE, TECH_PLASMA_WEB}},

	// TOPIC_POSITRONICS
	{900, 0, {TECH_HOLO_SIMULATOR, TECH_PLANETARY_SUPERCOMPUTER,
		TECH_POSITRONIC_COMPUTER}},
	{4500, 0, {TECH_CLASS_VII_SHIELD, TECH_PLANETARY_FLUX_SHIELD,
		TECH_WIDE_AREA_JAMMER}},
	{650, 0, {TECH_BATTLESTATION, TECH_POWERED_ARMOR, TECH_ROBOMINERS}},
	{900, 0, {TECH_ADVANCED_DAMAGE_CONTROL, TECH_ASSAULT_SHUTTLES,
		TECH_FAST_MISSILE_RACKS}},
	{2750, 0, {TECH_CLASS_V_SHIELD, TECH_GAUSS_CANNON,
		TECH_MULTIWAVE_ECM_JAMMER}},
	{1500, 0, {TECH_JUMP_GATE, TECH_SUBSPACE_COMMUNICATIONS}},
	{250, 0, {TECH_BATTLE_SCANNER, TECH_TACHYON_COMMUNICATIONS,
		TECH_TACHYON_SCANNER}},
	{3500, 0, {TECH_DEEP_CORE_MINING, TECH_CORE_WASTE_DUMPS}},
	{15000, 0, {TECH_CLASS_X_SHIELD, TECH_PHASING_CLOAK,
		TECH_PLANETARY_BARRIER_SHIELD}},
	{15000, 0, {TECH_STAR_GATE, TECH_STELLAR_CONVERTER,
		TECH_TIME_WARP_FACILITATOR}},

	// TOPIC_TRANS_GENETICS
	{7500, 0, {TECH_BIOMORPHIC_FUNGI, TECH_EVOLUTIONARY_MUTATION,
		TECH_GAIA_TRANSFORMATION}},
	{7500, 0, {TECH_DISPLACEMENT_DEVICE, TECH_INERTIAL_NULLIFIER,
		TECH_SUBSPACE_TELEPORTER}},
	{2000, 0, {TECH_LIGHTNING_FIELD, TECH_PULSAR, TECH_WARP_INTERDICTOR}},
	{650, 0, {TECH_ALIEN_MANAGEMENT_CENTER, TECH_XENO_PSYCHOLOGY}},
	{0}, // TOPIC_XENON_TECHNOLOGY, always unavailable

	// TOPIC_HYPER_BIOLOGY
	{25000, 0, {TECH_HYPER_BIOLOGY}},
	{25000, 0, {TECH_HYPER_POWER}},
	{25000, 0, {TECH_HYPER_PHYSICS}},
	{25000, 0, {TECH_HYPER_CONSTRUCTION}},
	{25000, 0, {TECH_HYPER_FIELDS}},
	{25000, 0, {TECH_HYPER_CHEMISTRY}},
	{25000, 0, {TECH_HYPER_COMPUTERS}},
	{25000, 0, {TECH_HYPER_SOCIOLOGY}},
};

TechListWidget::TechListGroup::TechListGroup(void) : title(NULL), itemCount(0),
	height(0), color(FONT_COLOR_DEFAULT), items(NULL) {

}

TechListWidget::TechListGroup::~TechListGroup(void) {
	clear();
}

void TechListWidget::TechListGroup::clear(void) {
	unsigned i;

	for (i = 0; i < itemCount; i++) {
		delete[] items[i].name;
	}

	itemCount = 0;
	delete[] items;
	delete[] title;
	items = NULL;
	title = NULL;
	height = 0;
}

TechListWidget::TechListWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, unsigned titleFont, unsigned itemFont) :
	Widget(x, y, width, height), _curGroup(-1), _curItem(-1),
	_selGroup(-1), _selItem(-1), _groupCount(0), _maxGroups(16),
	_curPage(0), _pageCount(1), _maxPages(16), _titleFont(titleFont),
	_itemFont(itemFont), _pages(NULL), _groups(NULL) {

	_groups = new TechListGroup*[_maxGroups];

	try {
		_pages = new unsigned[_maxPages + 1];
	} catch (...) {
		delete[] _groups;
		throw;
	}

	memset(_pages, 0, (_maxPages + 1) * sizeof(unsigned));
}

TechListWidget::~TechListWidget(void) {
	clear();
	delete[] _groups;
	delete[] _pages;
}

int TechListWidget::updateHighlight(int x, int y) {
	unsigned i;
	int oldgroup = _curGroup, olditem = _curItem, ypos = 2;
	int iheight, theight, tmp, newgroup = -1, newitem = -1;
	Font *fnt, *titlefnt;

	fnt = gameFonts->getFont(_itemFont);
	titlefnt = gameFonts->getFont(_titleFont);
	iheight = fnt->height() + TECHLIST_ITEM_SPACING;
	theight = titlefnt->height() + TECHLIST_TITLE_SPACING;

	for (i = _pages[_curPage]; i < _pages[_curPage + 1]; i++) {
		if (y < ypos) {
			break;
		}

		tmp = y - ypos - theight;

		if (tmp < (int)_groups[i]->itemCount * iheight) {
			newgroup = i;
			newitem = tmp > 0 ? tmp / iheight : 0;
			break;
		}

		ypos += _groups[i]->height + TECHLIST_GROUP_SPACING;
	}

	if (oldgroup != newgroup || olditem != newitem) {
		highlightItem(newgroup, newitem);
		return 1;
	}

	return 0;
}

void TechListWidget::clear(void) {
	unsigned i, count = _groupCount;

	_curGroup = -1;
	_curItem = -1;
	_selGroup = -1;
	_selItem = -1;
	_curPage = 0;
	_pageCount = 1;
	_pages[1] = 0;
	_groupCount = 0;

	for (i = 0; i < count; i++) {
		delete _groups[i];
	}
}

void TechListWidget::addGroup(const char *title, unsigned color,
	const TechListItem *items, unsigned itemCount) {
	unsigned i, newsize, ypos;
	TechListGroup **tmp, **ptr, *group;
	Font *fnt, *titlefnt;

	if (_groupCount >= _maxGroups) {
		newsize = _maxGroups * 2;
		ptr = new TechListGroup*[newsize];
		memcpy(ptr, _groups, _groupCount * sizeof(TechListGroup*));
		tmp = _groups;
		_groups = ptr;
		delete[] tmp;
		_maxGroups = newsize;
	}

	fnt = gameFonts->getFont(_itemFont);
	titlefnt = gameFonts->getFont(_titleFont);
	_groups[_groupCount] = group = new TechListGroup;

	try {
		group->title = copystr(title);
		group->color = color;
		group->height = titlefnt->height() + TECHLIST_TITLE_SPACING +
			itemCount * (fnt->height() + TECHLIST_ITEM_SPACING);
		group->items = new TechListItem[itemCount];
		memset(group->items, 0, itemCount * sizeof(TechListItem));
		group->itemCount = itemCount;

		for (i = 0; i < itemCount; i++) {
			group->items[i].tech_id = items[i].tech_id;
			group->items[i].color = items[i].color;
			group->items[i].name = copystr(items[i].name);
		}
	} catch (...) {
		delete group;
		throw;
	}

	for (i = _pages[_pageCount - 1], ypos = 2; i <= _groupCount; i++) {
		ypos += _groups[i]->height + TECHLIST_GROUP_SPACING;
	}

	if (ypos < height() || !_groupCount) {
		_pages[_pageCount]++;
		_groupCount++;
		return;
	}

	if (_pageCount >= _maxPages) {
		unsigned *ptmp, *pptr;

		newsize = _maxPages * 2;

		try {
			pptr = new unsigned[newsize + 1];
		} catch (...) {
			delete group;
			throw;
		}

		memcpy(pptr, _pages, (_pageCount + 1) * sizeof(unsigned));
		ptmp = _pages;
		_pages = pptr;
		delete[] ptmp;
		_maxPages = newsize;
	}

	_pages[_pageCount + 1] = _groupCount + 1;
	_pageCount++;
	_groupCount++;
}

void TechListWidget::highlightItem(int group, int item) {
	if (group < 0 || item < 0) {
		_curGroup = _curItem = -1;
	}

	if (unsigned(group) >= _groupCount ||
		unsigned(item) >= _groups[group]->itemCount) {
		return;
	}

	_curGroup = group;
	_curItem = item;
}

int TechListWidget::highlightedGroup(void) const {
	return _curGroup;
}

int TechListWidget::highlightedItem(void) const {
	return _curItem;
}

Technology TechListWidget::highlightedTechID(void) const {
	if (_curGroup < 0 || _curItem < 0) {
		return TECH_NONE;
	}

	return _groups[_curGroup]->items[_curItem].tech_id;
}

void TechListWidget::selectItem(int group, int item) {
	if (group < 0 || item < 0) {
		_selGroup = _selItem = -1;
	}

	if (unsigned(group) >= _groupCount ||
		unsigned(item) >= _groups[group]->itemCount) {
		return;
	}

	_selGroup = group;
	_selItem = item;
}

int TechListWidget::selectedGroup(void) const {
	return _selGroup;
}

int TechListWidget::selectedItem(void) const {
	return _selItem;
}

Technology TechListWidget::selectedTechID(void) const {
	if (_selGroup < 0 || _selItem < 0) {
		return TECH_NONE;
	}

	return _groups[_selGroup]->items[_selItem].tech_id;
}

unsigned TechListWidget::currentPage(void) const {
	return _curPage;
}

unsigned TechListWidget::pageCount(void) const {
	return _pageCount;
}

void TechListWidget::setPage(unsigned page) {
	if (page >= _pageCount) {
		return;
	}

	_curPage = page;
	highlightItem(-1, -1);
	selectItem(-1, -1);
}

void TechListWidget::previousPage(void) {
	if (_curPage > 0) {
		setPage(_curPage - 1);
	}
}

void TechListWidget::nextPage(void) {
	setPage(_curPage + 1);
}

void TechListWidget::setItemHighlightCallback(const GuiCallback &callback) {
	_onHighlightItem = callback;
}

void TechListWidget::setItemSelectionCallback(const GuiCallback &callback) {
	_onSelectItem = callback;
}

void TechListWidget::setItemExamineCallback(const GuiCallback &callback) {
	_onExamineItem = callback;
}

void TechListWidget::handleMouseMove(int x, int y, unsigned buttons) {
	if (updateHighlight(x - getX(), y - getY())) {
		_onHighlightItem(x, y);
		return;
	}

	Widget::handleMouseMove(x, y, buttons);
}

void TechListWidget::handleMouseUp(int x, int y, unsigned button) {
	if (updateHighlight(x - getX(), y - getY())) {
		_onHighlightItem(x, y);
	}

	if (button == MBUTTON_LEFT && _curItem >= 0) {
		_selGroup = _curGroup;
		_selItem = _curItem;
		_onSelectItem(x, y);
		return;
	} else if (button == MBUTTON_RIGHT && _curItem >= 0) {
		_onExamineItem(x, y);
		return;
	}

	Widget::handleMouseUp(x, y, button);
}

void TechListWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, j, color, selcolor, maxw = width();
	int ypos, tmpy;
	Font *fnt, *titlefnt;
	const uint8_t *pal;

	fnt = gameFonts->getFont(_itemFont);
	titlefnt = gameFonts->getFont(_titleFont);
	x += getX();
	y += getY() + 2;
	// TODO: animated color
	selcolor = FONT_COLOR_RESEARCH_BRIGHT;
	pal = Font::fontPalette(selcolor);

	for (i = _pages[_curPage]; i < _pages[_curPage + 1]; i++) {
		if (_curGroup == (int)i || _selGroup == (int)i) {
			color = selcolor;
		} else {
			color = _groups[i]->color;
		}

		fitText(x + 2, y, maxw - 4, _titleFont, color,
			_groups[i]->title, OUTLINE_NONE, 2);
		y += titlefnt->height() + TECHLIST_TITLE_SPACING;

		for (j = 0, ypos = y; j < _groups[i]->itemCount; j++) {
			if ((_curGroup == (int)i && _curItem == (int)j) ||
				(_selGroup == (int)i && _selItem == (int)j)) {
				color = selcolor;
			} else {
				color = _groups[i]->items[j].color;
			}

			fitText(x + 12, ypos, maxw - 14, _itemFont, color,
				_groups[i]->items[j].name, OUTLINE_NONE, 2);
			ypos += fnt->height() + TECHLIST_ITEM_SPACING;

			if (_selGroup == (int)i && _selItem == (int)j) {
				tmpy = ypos - 3 - fnt->height() / 2;
				gameScreen->drawLine(x + 4, y - 3, x + 4, tmpy,
					pal[9], pal[10], pal[11]);
				gameScreen->drawLine(x + 5, tmpy, x + 10, tmpy,
					pal[9], pal[10], pal[11]);
				gameScreen->drawLine(x + 7, tmpy - 3, x + 9,
					tmpy - 1, pal[9], pal[10], pal[11]);
				gameScreen->drawLine(x + 7, tmpy + 3, x + 9,
					tmpy + 1, pal[9], pal[10], pal[11]);
			}
		}

		y = ypos + TECHLIST_GROUP_SPACING;
	}
}

ResearchSelectWidget::ResearchSelectWidget(GuiView *parent, int x, int y,
	unsigned width, unsigned height, GameState *game, int activePlayer,
	unsigned area) : Widget(x, y, width, height), _parent(parent),
	_game(game), _activePlayer(activePlayer), _highlight(-1),
	_selection(-1), _choiceCount(0), _startTick(0) {

	unsigned i;
	Technology tech;
	Player *pptr;

	if (area >= MAX_RESEARCH_AREAS) {
		throw std::out_of_range("Invalid research area");
	}

	if (activePlayer < 0 || activePlayer >= _game->_playerCount) {
		throw std::out_of_range("Invalid player");
	}

	pptr = _game->_players + activePlayer;

	for (i = 0; !isHyperTopic(techtree[area][i]); i++) {
		if (pptr->canResearchTopic(techtree[area][i])) {
			break;
		}
	}

	_topic = techtree[area][i];

	for (i = 0; i < MAX_RESEARCH_CHOICES; i++) {
		tech = research_choices[_topic].choices[i];

		if (!tech) {
			break;
		}
	
		if (!isHyperTopic(_topic)) {
			if (pptr->knowsTechnology(tech) ||
				!pptr->canResearchTech(tech)) {
				continue;
			}

			if (pptr->researchItem == (uint16_t)tech) {
				_selection = _choiceCount;
			}
		}

		_choices[_choiceCount++] = tech;
	}

	if (!_choiceCount) {
		_choices[_choiceCount++] = TECH_NONE;
	}
}

ResearchSelectWidget::~ResearchSelectWidget(void) {

}

unsigned ResearchSelectWidget::findChoice(int y) const {
	Font *fnt, *titlefnt;
	unsigned ret;

	titlefnt = gameFonts->getFont(FONTSIZE_BIG);
	fnt = gameFonts->getFont(FONTSIZE_MEDIUM);
	y -= getY() + titlefnt->height() + 4;
	ret = y > 0 ? y / (fnt->height() + 2) : 0;
	return ret >= _choiceCount ? _choiceCount - 1 : ret;
}

int ResearchSelectWidget::isSelected(void) const {
	return _game->_players[_activePlayer].researchTopic == (uint8_t)_topic;
}

int ResearchSelectWidget::isHighlighted(void) const {
	return _highlight >= 0;
}

unsigned ResearchSelectWidget::researchCost(int full) const {
	return _game->_players[_activePlayer].researchCost(_topic, full);
}

void ResearchSelectWidget::setHighlightCallback(const GuiCallback &callback) {
	_onHighlightChange = callback;
}

void ResearchSelectWidget::setSelectCallback(const GuiCallback &callback) {
	_onSelectionChange = callback;
}

void ResearchSelectWidget::handleMouseMove(int x, int y, unsigned buttons) {
	int old_id = _highlight;

	_highlight = findChoice(y);

	if (old_id != _highlight) {
		if (old_id < 0) {
			_startTick = 0;
		}

		_onHighlightChange(x, y);
	}
}

void ResearchSelectWidget::handleMouseOut(int x, int y, unsigned buttons) {
	_highlight = -1;
	_onHighlightChange(x, y);
}

void ResearchSelectWidget::handleMouseUp(int x, int y, unsigned button) {
	unsigned idx = findChoice(y);

	if (button == MBUTTON_LEFT) {
		_selection = idx;
		_onSelectionChange(x, y);
		return;
	} else if (button == MBUTTON_RIGHT && _choices[idx] != TECH_NONE) {
		new MessageBoxWindow(_parent, _choices[idx], researchCost(1));
		return;
	}

	Widget::handleMouseUp(x, y, button);
}

void ResearchSelectWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, spacing = 2, color = FONT_COLOR_RESEARCH_NORMAL;
	Font *fnt, *titlefnt;
	const char *str;
	const Player *pptr;
	StringBuffer buf;

	if (_highlight >= 0 && !_startTick) {
		_startTick = curtick;
	}

	titlefnt = gameFonts->getFont(FONTSIZE_BIG);
	fnt = gameFonts->getFont(FONTSIZE_MEDIUM);
	x += getX();
	y += getY();
	pptr = _game->_players + _activePlayer;

	if (_highlight >= 0) {
		color = FONT_COLOR_RESEARCH_BRIGHT;
	} else if (pptr->researchTopic == (uint8_t)_topic) {
		color = FONT_COLOR_RESEARCH_BRIGHTER;
	}

	if (isHyperTopic(_topic)) {
		str = gameLang->estrings(ESTR_RTOPIC_HYPER);
	} else {
		str = gameLang->techname(TNAME_RTOPIC_STARTING_TECH + _topic);
	}

	while (spacing && titlefnt->textWidth(str, spacing) > width() - 2) {
		spacing--;
	}

	titlefnt->renderText(x + 2, y + 3, color, str, OUTLINE_NONE, spacing);

	if (_highlight >= 0) {
		int y2 = y + titlefnt->height() + 4;
		unsigned lastArrow;
		const uint8_t *pix;

		color = bounceFrame(curtick - _startTick, TECH_HIGHLIGHT_SPEED,
			TECH_HIGHLIGHT_FRAMECOUNT);
		pix = techHighlightColors + 3 * color;
		gameScreen->drawRect(x, y, width(), height(), pix[0], pix[1],
			pix[2]);
		lastArrow = i = _highlight;

		if (research_choices[_topic].research_all ||
			pptr->traits.creative) {
			i = 0;
			lastArrow = _choiceCount - 1;
		}

		gameScreen->drawLine(x + 3, y2, x + 3,
			y2 + lastArrow * (fnt->height() + 2) + 8, pix[0],
			pix[1], pix[2]);
		y2 += i * (fnt->height() + 2) + 8;

		for (; i <= lastArrow; i++) {
			gameScreen->drawLine(x + 3, y2, x + 10, y2, pix[0],
				pix[1], pix[2]);
			gameScreen->drawLine(x + 7, y2 - 3, x + 9, y2 - 1,
				pix[0], pix[1], pix[2]);
			gameScreen->drawLine(x + 7, y2 + 3, x + 9, y2 + 1,
				pix[0], pix[1], pix[2]);
			y2 += fnt->height() + 2;
		}
	}

	y += titlefnt->height() + 7;

	for (i = 0; i < _choiceCount; i++) {
		color = FONT_COLOR_RESEARCH_NORMAL;

		if (_highlight == (int)i) {
			color = FONT_COLOR_RESEARCH_BRIGHT;
		} else if (_selection == (int)i ||
			(pptr->researchTopic == _topic &&
			!isHyperTopic(_topic) && (pptr->traits.creative ||
			research_choices[_topic].research_all))) {

			color = FONT_COLOR_RESEARCH_BRIGHTER;
		}

		if (_choices[i] == TECH_NONE) {
			str = gameLang->misctext(TXT_MISC_BILLTEXT,
				BILL_PURE_RESEARCH);
		} else {
			str = gameLang->techname(TNAME_TECH_NONE + _choices[i]);

			if (isHyperTopic(_topic)) {
				StringBuffer num;

				num.roman(pptr->knowsTechnology(_choices[i]) +
					1);
				buf.printf("%s %s", str, num.c_str());
				str = buf.c_str();
			}
		}

		fnt->renderText(x + 12, y, color, str, OUTLINE_NONE, 2);
		y += fnt->height() + 2;
	}

}

ResearchSelectWindow::ResearchSelectWindow(GuiView *parent, GameState *game,
	int activePlayer, int allow_cancel) : GuiWindow(parent, WINDOW_MODAL),
	_game(game), _activePlayer(activePlayer) {

	ImageAsset palImg;

	if (allow_cancel) {
		palImg = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
		_bg = gameAssets->getImage(TECHSEL_ARCHIVE,
			ASSET_TECHSEL_BG + TECHSEL_CANCEL_OFFSET,
			palImg->palette());
	} else {
		palImg = gameAssets->getImage(SCIENCE_ARCHIVE,
			ASSET_SCIENCE_CURSOR);
		_bg = gameAssets->getImage(TECHSEL_ARCHIVE, ASSET_TECHSEL_BG,
			palImg->palette());
	}

	_width = _bg->width();
	_height = _bg->height();
	_x = (SCREEN_WIDTH - _width) / 2;
	_y = (SCREEN_HEIGHT - _height) / 2;

	initWidgets(allow_cancel);
}

ResearchSelectWindow::~ResearchSelectWindow(void) {

}

void ResearchSelectWindow::initWidgets(int allow_cancel) {
	Widget *w;
	unsigned i, x, off = allow_cancel ? TECHSEL_CANCEL_OFFSET : 0;
	const uint8_t *pal = _bg->palette();
	const int ypos[MAX_RESEARCH_AREAS / 2] = {45, 149, 254, 361};

	for (i = 0; i < MAX_RESEARCH_AREAS; i++) {
		ImageAsset img = gameAssets->getImage(TECHSEL_ARCHIVE,
			ASSET_TECHSEL_AREA_BUTTONS + off + i, pal);

		x = i % 2 ? 248 : 21;
		w = createWidget(x, ypos[i / 2] - img->height(),
			(Image*)img, 1);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod(*this, &ResearchSelectWindow::clickTechArea,
				i));

		x = i % 2 ? 240 : 13;
		_techChoices[i] = new ResearchSelectWidget(_parent, x,
			ypos[i / 2] + 4, 218, 82, _game, _activePlayer,
			area_list[i]);
		addWidget(_techChoices[i]);
		_techChoices[i]->setHighlightCallback(GuiMethod(*this,
			&ResearchSelectWindow::techHighlightChanged, i));
		_techChoices[i]->setSelectCallback(GuiMethod(*this,
			&ResearchSelectWindow::techSelected, i));

		x = i % 2 ? 368 : 141;
		_costLabels[i] = new LabelWidget(x, ypos[i / 2] - 13, 86, 14);
		addWidget(_costLabels[i]);
		techHighlightChanged(0, 0, i);
	}

	if (allow_cancel) {
		w = createWidget(189, 452, TECHSEL_ARCHIVE,
			ASSET_TECHSEL_CANCEL_BUTTON, pal, 1);
		w->setMouseUpCallback(MBUTTON_LEFT,
			GuiMethod<ResearchSelectWindow>(*this,
			&ResearchSelectWindow::close));
	}
}

void ResearchSelectWindow::techHighlightChanged(int x, int y, int arg) {
	unsigned color = FONT_COLOR_RESEARCH_NORMAL;
	StringBuffer buf;

	if (_techChoices[arg]->isSelected()) {
		color = FONT_COLOR_RESEARCH_BRIGHTER;
	} else if (_techChoices[arg]->isHighlighted()) {
		color = FONT_COLOR_RESEARCH_BRIGHT;
	}

	buf.printf("%u RP", _techChoices[arg]->researchCost());
	// FIXME: set char spacing = 2
	_costLabels[arg]->setText(buf.c_str(), FONTSIZE_BIG, color,
		OUTLINE_NONE, ALIGN_RIGHT);
}

void ResearchSelectWindow::techSelected(int x, int y, int arg) {
	new MessageBoxWindow(_parent, "Not implemented yet");
	close();
}

void ResearchSelectWindow::clickTechArea(int x, int y, int arg) {
	new ResearchListWindow(_parent, _game, _activePlayer, area_list[arg]);
}

void ResearchSelectWindow::redraw(unsigned curtick) {
	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);
}

ResearchListWindow::ResearchListWindow(GuiView *parent, GameState *game,
	int activePlayer, ResearchArea area, unsigned flags) :
	GuiWindow(parent, flags), _game(game), _topicOffset(0),
	_activePlayer(activePlayer), _area(area) {

	ImageAsset palImg1, palImg2;

	palImg1 = gameAssets->getImage(GALAXY_ARCHIVE, ASSET_GALAXY_GUI);
	palImg2 = gameAssets->getImage(TECHSEL_ARCHIVE,
			ASSET_TECHSEL_BG + TECHSEL_CANCEL_OFFSET,
			palImg1->palette());
	_bg = gameAssets->getImage(TECHSEL_ARCHIVE, ASSET_TECHLIST_BG,
		palImg2->palette());
	_width = _bg->width();
	_height = _bg->height();
	_x = (SCREEN_WIDTH - _width) / 2;
	_y = (SCREEN_HEIGHT - _height) / 2;

	initWidgets();
	initList();
}

ResearchListWindow::~ResearchListWindow(void) {

}

void ResearchListWindow::initWidgets(void) {
	Widget *w;
	const uint8_t *pal = _bg->palette();

	_list = new TechListWidget(14, 40, 228, 354, FONTSIZE_BIG,
		FONTSIZE_MEDIUM);
	addWidget(_list);
	_list->setMouseOutCallback(GuiMethod(*this,
		&ResearchListWindow::clearHighlight));
	_list->setItemSelectionCallback(GuiMethod(*this,
		&ResearchListWindow::clearSelection));
	_list->setItemExamineCallback(GuiMethod(*this,
		&ResearchListWindow::showTechHelp));

	_buttonUp = w = createWidget(247, 46, TECHSEL_ARCHIVE,
		ASSET_TECHLIST_UP_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&ResearchListWindow::previousPage));

	_buttonDown = w = createWidget(248, 367, TECHSEL_ARCHIVE,
		ASSET_TECHLIST_DOWN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT, GuiMethod(*this,
		&ResearchListWindow::nextPage));

	w = createWidget(187, 403, TECHSEL_ARCHIVE,
		ASSET_TECHLIST_RETURN_BUTTON, pal, 1);
	w->setMouseUpCallback(MBUTTON_LEFT,
		GuiMethod<ResearchListWindow>(*this,
		&ResearchListWindow::close));
}

void ResearchListWindow::initList(void) {
	unsigned i, j, count = 0, itemcolor, color = FONT_COLOR_RESEARCH_NORMAL;
	int active;
	Technology tech;
	const ResearchChoice *techgroup;
	const Player *pptr = _game->_players + _activePlayer;
	const char *str;
	TechListWidget::TechListItem items[MAX_RESEARCH_CHOICES];
	StringBuffer buf, num;

	for (i = 0; !isHyperTopic(techtree[_area][i]); i++) {
		if (pptr->canResearchTopic(techtree[_area][i])) {
			break;
		}
	}

	_topicOffset = i;

	for (; !isHyperTopic(techtree[_area][i]); i++) {
		techgroup = research_choices + techtree[_area][i];
		active = 0;

		if (pptr->researchTopic == (unsigned)techtree[_area][i]) {
			color = FONT_COLOR_RESEARCH_BRIGHTER;
			active = 1;
		}

		try {
			for (j = 0, count = 0; j < MAX_RESEARCH_CHOICES; j++) {
				tech = techgroup->choices[j];

				if (!tech) {
					break;
				}

				itemcolor = color;

				if (pptr->knowsTechnology(tech)) {
					itemcolor = FONT_COLOR_RESEARCH_LIGHT;
				} else if (!pptr->canResearchTech(tech)) {
					continue;
				}

				str = gameLang->techname(TNAME_TECH_NONE+tech);
				buf.printf("^%s", str);
				items[count].tech_id = tech;
				items[count].color = active ? color : itemcolor;
				items[count].name = copystr(buf.c_str());
				count++;
			}

			str = gameLang->techname(TNAME_RTOPIC_STARTING_TECH +
				techtree[_area][i]);
			_list->addGroup(str, color, items, count);
		} catch (...) {
			for (j = 0; j < count; j++) {
				delete[] items[j].name;
			}

			throw;
		}

		for (j = 0; j < count; j++) {
			delete[] items[j].name;
		}

		color = FONT_COLOR_RESEARCH_LIGHT;
	}

	// Add the hyper-advanced tech
	if (pptr->researchTopic == (unsigned)techtree[_area][i]) {
		color = FONT_COLOR_RESEARCH_BRIGHTER;
	}

	techgroup = research_choices + techtree[_area][i];
	items[0].tech_id = tech = techgroup->choices[0];
	items[0].color = color;
	str = gameLang->techname(TNAME_TECH_NONE + tech);
	num.roman(pptr->knowsTechnology(tech) + 1);
	buf.printf("^%s %s", str, num.c_str());
	items[0].name = copystr(buf.c_str());

	try {
		str = gameLang->estrings(ESTR_RTOPIC_HYPER);
		_list->addGroup(str, color, items, 1);
	} catch (...) {
		delete[] items[0].name;
		throw;
	}

	delete[] items[0].name;
	enablePageButtons();
}

void ResearchListWindow::enablePageButtons(void) {
	unsigned page = _list->currentPage();

	_buttonUp->disable(!page);
	_buttonDown->disable(page + 1 >= _list->pageCount());
}

void ResearchListWindow::showTechHelp(int x, int y, int arg) {
	unsigned cost, group, item, topic;
	Technology tech = _list->highlightedTechID();
	GuiWindow *msg;

	if (tech) {
		group = _list->highlightedGroup();
		item = _list->highlightedItem();
		topic = techtree[_area][_topicOffset + group];
		cost = _game->_players[_activePlayer].researchCost(topic, 1);
		_list->selectItem(group, item);
		_list->highlightItem(-1, -1);
		msg = new MessageBoxWindow(_parent, tech, cost);
		msg->setCloseCallback(GuiMethod(*this,
			&ResearchListWindow::clearSelection, 1));
	}
}

void ResearchListWindow::clearHighlight(int x, int y, int arg) {
	_list->highlightItem(-1, -1);
}

void ResearchListWindow::clearSelection(int x, int y, int arg) {
	_list->selectItem(-1, -1);

	if (arg && isInRect(x, y, _x, _y, _width, _height)) {
		handleMouseOver(x, y, 0);
		handleMouseMove(x, y, 0);
	}
}

void ResearchListWindow::previousPage(int x, int y, int arg) {
	_list->previousPage();
	enablePageButtons();
}

void ResearchListWindow::nextPage(int x, int y, int arg) {
	_list->nextPage();
	enablePageButtons();
}

void ResearchListWindow::redraw(unsigned curtick) {
	int x;
	const char *title, *str;
	Font *fnt;

	fnt = gameFonts->getFont(FONTSIZE_BIG);
	title = gameLang->misctext(TXT_MISC_BILLTEXT,
		BILL_RESEARCH_TOPIC_BIOLOGY + _area);
	str = gameLang->misctext(TXT_MISC_BILLTEXT, BILL_RESEARCH_TOPIC_LIST);
	x = _x + _width / 2;
	x -= (fnt->textWidth(title, 2) + fnt->textWidth(str, 2) + 2) / 2 + 4;
	_bg->draw(_x, _y);
	x = fnt->renderText(x, _y + 16, FONT_COLOR_RESEARCH_NORMAL, title,
		OUTLINE_NONE, 2);
	fnt->renderText(x, _y + 16, FONT_COLOR_RESEARCH_NORMAL, str,
		OUTLINE_NONE, 2);

	redrawWidgets(_x, _y, curtick);
}

int isHyperTopic(unsigned topic) {
	return topic >= TOPIC_HYPER_BIOLOGY;
}
