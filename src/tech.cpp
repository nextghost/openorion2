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
#define TECHSEL_CANCEL_OFFSET 14

#define MAX_AREA_TOPICS 14

#define TECH_HIGHLIGHT_FRAMECOUNT 6
#define TECH_HIGHLIGHT_SPEED 80

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

const ResearchChoice research_choices[MAX_APPLIED_TECHS] = {
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
		drawRect(x, y, width(), height(), pix[0], pix[1], pix[2]);
		lastArrow = i = _highlight;

		if (research_choices[_topic].research_all ||
			pptr->traits.creative) {
			i = 0;
			lastArrow = _choiceCount - 1;
		}

		drawLine(x + 3, y2, x + 3,
			y2 + lastArrow * (fnt->height() + 2) + 8, pix[0],
			pix[1], pix[2]);
		y2 += i * (fnt->height() + 2) + 8;

		for (; i <= lastArrow; i++) {
			drawLine(x + 3, y2, x + 10, y2, pix[0], pix[1], pix[2]);
			drawLine(x + 7, y2 - 3, x + 9, y2 - 1, pix[0], pix[1],
				pix[2]);
			drawLine(x + 7, y2 + 3, x + 9, y2 + 1, pix[0], pix[1],
				pix[2]);
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
				// TODO: Roman numerals
				buf.printf("%s %d", str,
					pptr->knowsTechnology(_choices[i]) + 1);
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
		w = createWidget(x, ypos[i / 2] - img->height(), img->width(),
			img->height());
		w->setClickSprite(MBUTTON_LEFT, (Image*)img, 1);
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
		w = createWidget(189, 452, 92, 19);
		w->setClickSprite(MBUTTON_LEFT, TECHSEL_ARCHIVE,
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

void ResearchSelectWindow::clickTechArea(int x, int y, int arg) STUB(_parent)

void ResearchSelectWindow::redraw(unsigned curtick) {
	_bg->draw(_x, _y);
	redrawWidgets(_x, _y, curtick);
}

int isHyperTopic(unsigned topic) {
	return topic >= TOPIC_HYPER_BIOLOGY;
}
