/*
 * This file is part of OpenOrion2
 * Copyright (C) 2021 Daniel Donisa, Martin Doucha
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
#include <stdexcept>
#include "gamestate.h"

const int LEADERS_DATA_OFFSET 		= 0x019a9b;
const int STARS_COUNT_DATA_OFFSET 	= 0x017ad1;

GameConfig::GameConfig(void) {
	version = 0;
	memset(saveGameName, 0, SAVE_GAME_NAME_SIZE);
	stardate = 0;
	multiplayer = 0;
	endOfTurnSummary = 0;
	endOfTurnWait = 0;
	randomEvents = 0;
	enemyMoves = 0;
	expandingHelp = 0;
	autoSelectShips = 0;
	animations = 0;
	autoSelectColony = 0;
	showRelocationLines = 0;
	showGNNReport = 0;
	autoDeleteTradeGoodHousing = 0;
	showOnlySeriousTurnSummary = 0;
	shipInitiative = 0;
}

void GameConfig::load(ReadStream &stream) {
	version = stream.readUint32LE();
	stream.read(saveGameName, SAVE_GAME_NAME_SIZE);
	saveGameName[SAVE_GAME_NAME_SIZE - 1] = '\0';
	stardate = stream.readUint32LE();
	multiplayer = stream.readUint8();
	endOfTurnSummary = stream.readUint8();
	endOfTurnWait = stream.readUint8();
	randomEvents = stream.readUint8();
	enemyMoves = stream.readUint8();
	expandingHelp = stream.readUint8();
	autoSelectShips = stream.readUint8();
	animations = stream.readUint8();
	autoSelectColony = stream.readUint8();
	showRelocationLines = stream.readUint8();
	showGNNReport = stream.readUint8();
	autoDeleteTradeGoodHousing = stream.readUint8();
	showOnlySeriousTurnSummary = stream.readUint8();
	shipInitiative = stream.readUint8();
}

Nebula::Nebula(void) : x(0), y(0), type(0) {

}

void Nebula::load(ReadStream &stream) {
	x = stream.readUint16LE();
	y = stream.readUint16LE();
	type = stream.readUint8();

	if (type >= NEBULA_TYPE_COUNT) {
		throw std::runtime_error("Invalid nebula type");
	}
}

Galaxy::Galaxy(void) : sizeFactor(0), width(0), height(0), nebulaCount(0) {

}

void Galaxy::load(ReadStream &stream) {
	sizeFactor = stream.readUint8();
	stream.readUint32LE(); // Skip unknown data
	width = stream.readUint16LE();
	height = stream.readUint16LE();
	stream.readUint16LE(); // Skip unknown data

	for (int i = 0; i < MAX_NEBULAS; i++) {
		nebulas[i].load(stream);
	}

	nebulaCount = stream.readUint8();

	if (nebulaCount > MAX_NEBULAS) {
		throw std::runtime_error("Invalid nebula count");
	}
}

Leader::Leader(void) {
	int i;

	memset(name, 0, LEADER_NAME_SIZE);
	memset(title, 0, LEADER_TITLE_SIZE);
	type = 0;
	experience = 0;
	commonSkills = 0;
	specialSkills = 0;

	for (i = 0; i < MAX_LEADER_TECH_SKILLS; i++) {
		techs[i] = 0;
	}

	picture = 0;
	skillValue = 0;
	level = 0;
	location = -1;
	eta = 0;
	displayLevelUp = 0;
	status = 0;
	playerIndex = 0;
}

void Leader::load(ReadStream &stream) {
	int i;

	stream.read(name, LEADER_NAME_SIZE);
	name[LEADER_NAME_SIZE - 1] = '\0';
	stream.read(title, LEADER_TITLE_SIZE);
	title[LEADER_TITLE_SIZE - 1] = '\0';
	type = stream.readUint8();
	experience = stream.readUint16LE();
	commonSkills = stream.readUint32LE();
	specialSkills = stream.readUint32LE();

	for (i = 0; i < MAX_LEADER_TECH_SKILLS; i++) {
		techs[i] = stream.readUint8();
	}

	picture = stream.readUint8();
	skillValue = stream.readUint16LE();
	level = stream.readUint8();
	location = stream.readSint16LE();
	eta = stream.readUint8();
	displayLevelUp = stream.readUint8();
	status = stream.readUint8();
	playerIndex = stream.readUint8();
}

void ShipWeapon::load(ReadStream &stream) {
	type = stream.readUint8();
	stream.readUint8(); // FIXME: unknown data (weapon type is uint16_t?)
	maxCount = stream.readUint8();
	workingCount = stream.readUint8();
	arc = stream.readUint8();
	beamMods = stream.readUint8();
	missileMods = stream.readUint8();
	ammo = stream.readUint8();
}

ShipDesign::ShipDesign(void) {
	memset(name, 0, SHIP_NAME_SIZE);
	size = 0;
	type = 0;
	shield = 0;
	drive = 0;
	speed = 0;
	computer = 0;
	armor = 0;
	memset(specials, 0, (MAX_SHIP_SPECIALS + 7) / 8);
	memset(weapons, 0, MAX_SHIP_WEAPONS * sizeof(ShipWeapon));
	picture = 0;
	builder = 0;
	cost = 0;
	combatSpeed = 0;
	buildDate = 0;
}

void ShipDesign::load(ReadStream &stream) {
	int i;

	stream.read(name, SHIP_NAME_SIZE);
	name[SHIP_NAME_SIZE - 1] = '\0';
	size = stream.readUint8();
	type = stream.readUint8();
	shield = stream.readUint8();
	drive = stream.readUint8();
	speed = stream.readUint8();
	computer = stream.readUint8();
	armor = stream.readUint8();
	stream.read(specials, (MAX_SHIP_SPECIALS + 7) / 8);

	for (i = 0; i < MAX_SHIP_WEAPONS; i++) {
		weapons[i].load(stream);
	}

	picture = stream.readUint8();
	builder = stream.readUint8();
	cost = stream.readUint16LE();
	combatSpeed = stream.readUint8();
	buildDate = stream.readUint16LE();
}

void RacePicks::load(ReadStream &stream) {
	government = stream.readUint8();
	population = stream.readSint8();
	farming = stream.readSint8();
	industry = stream.readSint8();
	science = stream.readSint8();
	money = stream.readSint8();
	shipDefense = stream.readSint8();
	shipAttack = stream.readSint8();
	groundCombat = stream.readSint8();
	spying = stream.readSint8();
	lowG = stream.readUint8();
	highG = stream.readUint8();
	aquatic = stream.readUint8();
	subterranean = stream.readUint8();
	largeHomeworld = stream.readUint8();
	richHomeworld = stream.readSint8();
	artifactsHomeworld = stream.readUint8();
	cybernetic = stream.readUint8();
	lithovore = stream.readUint8();
	repulsive = stream.readUint8();
	charismatic = stream.readUint8();
	uncreative = stream.readUint8();
	creative = stream.readUint8();
	tolerant = stream.readUint8();
	fantasticTraders = stream.readUint8();
	telepathic = stream.readUint8();
	lucky = stream.readUint8();
	omniscience = stream.readUint8();
	stealthyShips = stream.readUint8();
	transDimensional = stream.readUint8();
	warlord = stream.readUint8();
}

void SettlerInfo::load(ReadStream &stream) {
	BitStream data(stream);

	sourceColony = data.readBitsLE(8);
	destinationPlanet = data.readBitsLE(8);
	player = data.readBitsLE(4);
	eta = data.readBitsLE(4);
	job = data.readBitsLE(2);
	//data.readBitsLE(6);
}

Player::Player(void) {
	memset(name, 0, PLAYER_NAME_SIZE);
	memset(race, 0, PLAYER_RACE_SIZE);
	picture = 0;
	color = 0;
	personality = 0;
	objective = 0;

	homePlayerId = 0;
	networkPlayerId = 0;
	playerDoneFlags = 0;
	researchBreakthrough = 0;
	taxRate = 0;
	BC = 0;
	totalFreighters = 0;
	surplusFreighters = 0;
	commandPoints = 0;
	usedCommandPoints = 0;
	foodFreighted = 0;
	settlersFreighted = 0;

	totalPop = 0;
	foodProduced = 0;
	industryProduced = 0;
	researchProduced = 0;
	bcProduced = 0;

	surplusFood = 0;
	surplusBC = 0;

	totalMaintenance = 0;
	researchArea = ResearchArea::None;
	researchItem = 0;
}

void Player::load(SeekableReadStream &stream) {
	int i;

	stream.readUint8();	// FIXME: unknown data
	stream.read(name, PLAYER_NAME_SIZE);
	name[PLAYER_NAME_SIZE - 1] = '\0';
	stream.read(race, PLAYER_RACE_SIZE);
	race[PLAYER_RACE_SIZE - 1] = '\0';
	picture = stream.readUint8();
	color = stream.readUint8();
	// 100 = Human player
	personality = stream.readUint8();
	objective = stream.readUint8();

	homePlayerId = stream.readUint16LE();
	networkPlayerId = stream.readUint16LE();
	playerDoneFlags = stream.readUint8();
	stream.readUint16LE(); // Dead field
	researchBreakthrough = stream.readUint8();
	taxRate = stream.readUint8();
	BC = stream.readUint32LE();
	totalFreighters = stream.readUint16LE();
	surplusFreighters = stream.readUint16LE();
	commandPoints = stream.readUint16LE();
	usedCommandPoints = stream.readSint16LE();
	foodFreighted = stream.readUint16LE();
	settlersFreighted = stream.readUint16LE();

	for (i = 0; i < MAX_SETTLERS; i++) {
		settlers[i].load(stream);
	}

	totalPop = stream.readUint16LE();
	foodProduced = stream.readUint16LE();
	industryProduced = stream.readUint16LE();
	researchProduced = stream.readUint16LE();
	bcProduced = stream.readUint16LE();

	surplusFood = stream.readSint16LE();
	surplusBC = stream.readSint16LE();

	// FIXME: check maintenance offset in save file
	totalMaintenance = stream.readSint32LE();
	stream.seek(0x269, SEEK_CUR);
	researchArea = ResearchArea(stream.readUint8());
	researchItem = stream.readUint8();

	stream.readUint8();	// FIXME: Unknown data
	stream.readUint8();	// FIXME: Unknown data
	stream.readUint8();	// FIXME: Unknown data

	for (i = 0; i < MAX_PLAYER_BLUEPRINTS; i++) {
		blueprints[i].load(stream);
	}

	selectedBlueprint.load(stream);
	stream.seek(0x327, SEEK_CUR);
	racePicks.load(stream);
	stream.seek(0x5eb, SEEK_CUR);
}

Star::Star(void) {
	memset(name, 0, STARS_NAME_SIZE);
	x = 0;
	y = 0;
	size = StarSize::Large;
	owner = 0;
	pictureType = 0;
	spectralClass = SpectralClass::Blue;

	memset(lastPlanetSelected, 0, sizeof(lastPlanetSelected));
	memset(blackHoleBlocks, 0, sizeof(blackHoleBlocks));

	special = SpecialType::None;
	wormhole = 0;
	blockaded = 0;

	memset(blockadedBy, 0, sizeof(blockadedBy));

	visited = 0;
	justVisited = 0;
	ignoreColonyShips = 0;
	ignoreCombatShips = 0;
	colonizePlayer = 0;
	hasColony = 0;
	hasWarpFieldInterdictor = 0;
	nextWFIInList = 0;
	hasTachyon = 0;
	hasSubspace = 0;
	hasStargate = 0;
	hasJumpgate = 0;
	hasArtemisNet = 0;
	hasDimensionalPortal = 0;
	isStagepoint = 0;

	memset(officerIndex, 0, sizeof(officerIndex));
	memset(planetIndex, 0, sizeof(planetIndex));
	memset(relocateShipTo, 0, sizeof(relocateShipTo));
	memset(surrenderTo, 0, sizeof(surrenderTo));

	inNebula = 0;
	artifactsGaveApp = 0;

	// Initialize fleet lists
	_firstOrbitingFleet.insert_before(&_lastOrbitingFleet);
	_firstLeavingFleet.insert_before(&_lastLeavingFleet);
}

Star::~Star(void) {
	BilistNode<Fleet> *ptr, *next;

	ptr = _firstOrbitingFleet.next();
	_firstOrbitingFleet.unlink();

	for (; ptr && ptr != &_lastOrbitingFleet; ptr = next) {
		next = ptr->next();
		delete ptr->data;
		delete ptr;
	}

	ptr = _firstLeavingFleet.next();
	_firstLeavingFleet.unlink();

	for (; ptr && ptr != &_lastLeavingFleet; ptr = next) {
		next = ptr->next();
		delete ptr->data;
		delete ptr;
	}
}

void Star::load(ReadStream &stream) {
	int i;

	stream.read(name, STARS_NAME_SIZE);
	name[STARS_NAME_SIZE - 1] = '\0';
	x = stream.readUint16LE();
	y = stream.readUint16LE();
	size = stream.readUint8();
	owner = stream.readUint8();
	pictureType = stream.readUint8();
	spectralClass = stream.readUint8();

	if (size > StarSize::Small) {
		throw std::out_of_range("Invalid star size");
	}

	if (spectralClass > SpectralClass::BlackHole) {
		throw std::out_of_range("Invalid star spectral class");
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		lastPlanetSelected[i] = stream.readUint8();
	}

	for (i = 0; i < (MAX_STARS + 7)/8; i++) {
		blackHoleBlocks[i] = stream.readUint8();
	}

	special = SpecialType(stream.readUint8());
	wormhole = stream.readSint8();
	blockaded = stream.readUint8();

	for (i = 0; i < MAX_PLAYERS; i++) {
		blockadedBy[i] = stream.readUint8();
	}

	visited = stream.readUint8();
	justVisited = stream.readUint8();
	ignoreColonyShips = stream.readUint8();
	ignoreCombatShips = stream.readUint8();
	colonizePlayer = stream.readSint8();
	hasColony = stream.readUint8();
	hasWarpFieldInterdictor = stream.readUint8();
	nextWFIInList = stream.readUint8();
	hasTachyon = stream.readUint8();
	hasSubspace = stream.readUint8();
	hasStargate = stream.readUint8();
	hasJumpgate = stream.readUint8();
	hasArtemisNet = stream.readUint8();
	hasDimensionalPortal = stream.readUint8();
	isStagepoint = stream.readUint8();

	for (i = 0; i < MAX_PLAYERS; i++) {
		officerIndex[i] = stream.readUint8();
	}

	for (i = 0; i < MAX_PLANETS_PER_SYSTEM; i++) {
		planetIndex[i] = stream.readUint16LE();
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		relocateShipTo[i] = stream.readUint16LE();
	}

	stream.readUint8();
	stream.readUint8();
	stream.readUint8();

	for (i = 0; i < MAX_PLAYERS; i++) {
		surrenderTo[i] = stream.readUint8();
	}

	inNebula = stream.readUint8();
	artifactsGaveApp = stream.readUint8();
}

void Star::addFleet(Fleet *f) {
	unsigned status = f->getStatus();

	if (status != ShipState::InOrbit && status != ShipState::LeavingOrbit) {
		throw std::invalid_argument("Cannot add moving fleet to star");
	}

	if (status == ShipState::InOrbit) {
		_lastOrbitingFleet.insert(f);
	} else {
		_lastLeavingFleet.insert(f);
	}
}

BilistNode<Fleet> *Star::getOrbitingFleets(void) {
	return _firstOrbitingFleet.next();
}

BilistNode<Fleet> *Star::getLeavingFleets(void) {
	return _firstLeavingFleet.next();
}

const BilistNode<Fleet> *Star::getOrbitingFleets(void) const {
	return _firstOrbitingFleet.next();
}

const BilistNode<Fleet> *Star::getLeavingFleets(void) const {
	return _firstLeavingFleet.next();
}

Ship::Ship(void) {
	owner = 0;
	status = 0;
	star = 0;
	x = 0;
	y = 0;
	groupHasNavigator = 0;
	warpSpeed = 0;
	eta = 0;
	shieldDamage = 0;
	driveDamage = 0;
	computerDamage = 0;
	crewLevel = 0;
	crewExp = 0;
	officer = 0;
	memset(damagedSpecials, 0, (MAX_SHIP_SPECIALS + 7) / 8);
	armorDamage = 0;
	structureDamage = 0;
	mission = 0;
	justBuilt = 0;
}

void Ship::load(ReadStream &stream) {
	design.load(stream);
	owner = stream.readUint8();
	status = stream.readUint8();
	star = stream.readSint16LE();
	x = stream.readUint16LE();
	y = stream.readUint16LE();
	groupHasNavigator = stream.readUint8();
	warpSpeed = stream.readUint8();
	eta = stream.readUint8();
	shieldDamage = stream.readUint8();
	driveDamage = stream.readUint8();
	computerDamage = stream.readUint8();
	crewLevel = stream.readUint8();
	crewExp = stream.readUint16LE();
	officer = stream.readSint16LE();
	stream.read(damagedSpecials, (MAX_SHIP_SPECIALS + 7) / 8);
	armorDamage = stream.readUint16LE();
	structureDamage = stream.readUint16LE();
	mission = stream.readUint8();
	justBuilt = stream.readUint8();
}

unsigned Ship::getStarID(void) const {
	return star - (status <= ShipState::LeavingOrbit ? 500 * status : 0);
}

int Ship::isActive(void) const {
	return status <= ShipState::LeavingOrbit;
}

int Ship::exists(void) const {
	return status <= ShipState::UnderConstruction &&
		status != ShipState::Unknown && status != ShipState::Destroyed;
}

GameState::GameState(void) {
	_firstMovingFleet.insert_before(&_lastMovingFleet);
}

GameState::~GameState(void) {
	BilistNode<Fleet> *next, *ptr = _firstMovingFleet.next();

	// prevent array scans in removeFleet() called by fleet destructor
	_firstMovingFleet.unlink();

	for (; ptr && ptr != &_lastMovingFleet; ptr = next) {
		next = ptr->next();
		delete ptr->data;
		delete ptr;
	}
}

Fleet *GameState::findFleet(unsigned owner, unsigned status, unsigned x,
	unsigned y, unsigned star_id) {

	BilistNode<Fleet> *node;
	Fleet *f;
	Star *dest;
	unsigned old_star;

	if (star_id > _starSystemCount) {
		throw std::out_of_range("Invalid star ID");
	}

	switch (status) {
	case ShipState::InOrbit:
		node = _starSystems[star_id].getOrbitingFleets();
		dest = NULL;
		break;

	case ShipState::InTransit:
		node = _firstMovingFleet.next();
		dest = _starSystems + star_id;
		break;

	case ShipState::LeavingOrbit:
		old_star = findStar(x, y);
		node = _starSystems[old_star].getLeavingFleets();
		dest = _starSystems + star_id;
		break;

	default:
		return NULL;
	}

	for (; node; node = node->next()) {
		if (!node->data) {
			continue;
		}

		f = node->data;

		if (owner == f->getOwner() && x == f->getX() &&
			y == f->getY() && dest == f->getDestStar()) {
			return f;
		}
	}

	return NULL;
}

void GameState::createFleets(void) {
	unsigned i;
	Ship *ptr;
	Fleet *flt;

	for (i = 0, ptr = _ships; i < _shipCount; i++, ptr++) {
		if (!ptr->isActive()) {
			continue;
		}

		flt = findFleet(ptr->owner, ptr->status, ptr->x, ptr->y,
			ptr->getStarID());

		if (flt) {
			flt->addShip(i);
			continue;
		}

		flt = new Fleet(this, i);

		try {
			addFleet(flt);
		} catch (...) {
			delete flt;
			throw;
		}
	}
}

void GameState::addFleet(Fleet *flt) {
	if (flt->getStatus() != ShipState::InTransit) {
		flt->getOrbitedStar()->addFleet(flt);
		return;
	}

	_lastMovingFleet.insert(flt);
}

void GameState::removeFleet(Fleet *flt) {
	BilistNode<Fleet> *ptr = _firstMovingFleet.next();

	for (; ptr && ptr != &_lastMovingFleet; ptr = ptr->next()) {
		if (ptr->data && ptr->data == flt) {
			ptr->discard();
			return;
		}
	}
}

void GameState::load(SeekableReadStream &stream) {
	int i, tmp;

	// FIXME: get rid of seeks
	_gameConfig.load(stream);
	stream.seek(0x31be4, SEEK_SET);
	_galaxy.load(stream);
	stream.seek(STARS_COUNT_DATA_OFFSET, SEEK_SET);
	// FIXME: Uint16LE?
	_starSystemCount = stream.readUint8();
	stream.readUint8();

	for (i = 0; i < _starSystemCount; i++) {
		_starSystems[i].load(stream);
	}

	for (i = 0; i < _starSystemCount; i++) {
		Star *ptr = _starSystems + i;

		if (ptr->x >= _galaxy.width || ptr->y >= _galaxy.height) {
			throw std::out_of_range("Star outside galaxy area");
		}

		if (ptr->wormhole >= _starSystemCount) {
			throw std::out_of_range("Invalid wormhole index");
		}

		if (ptr->wormhole >= 0 &&
			_starSystems[ptr->wormhole].wormhole != i) {
			std::logic_error("One-way wormholes not allowed");
		}
	}

	stream.seek(LEADERS_DATA_OFFSET, SEEK_SET);

	for (i = 0; i < LEADER_COUNT; i++) {
		_leaders[i].load(stream);
	}

	_playerCount = stream.readUint16LE();

	for (i = 0; i < PLAYER_COUNT; i++) {
		_players[i].load(stream);
	}

	_shipCount = stream.readUint16LE();

	for (i = 0; i < MAX_SHIPS; i++) {
		_ships[i].load(stream);
	}

	for (i = 0; i < _shipCount; i++) {
		Ship *ptr = _ships + i;

		if (ptr->status == ShipState::Destroyed) {
			continue;
		}

		if (ptr->status > ShipState::UnderConstruction) {
			throw std::out_of_range("Invalid ship status");
		}

		if (ptr->x >= _galaxy.width || ptr->y >= _galaxy.height) {
			throw std::out_of_range("Ship outside galaxy area");
		}

		tmp = ptr->getStarID();

		if (tmp >= _starSystemCount &&
			(ptr->status != ShipState::LeavingOrbit ||
			tmp != _starSystemCount)) {

			throw std::out_of_range("Ship has invalid star ID");
		}

		if (ptr->officer >= LEADER_COUNT) {
			throw std::out_of_range("Ship has invalid officer");
		}
	}

	createFleets();
}

void GameState::load(const char *filename) {
	File fr;

	if (!fr.open(filename)) {
		throw std::runtime_error("Cannot open savegame file");
	}

	load(fr);
}

unsigned GameState::findStar(int x, int y) const {
	unsigned i;

	for (i = 0; i < _starSystemCount; i++) {
		if (_starSystems[i].x == x && _starSystems[i].y == y) {
			return i;
		}
	}

	throw std::runtime_error("No star at given coordinates");
}

BilistNode<Fleet> *GameState::getMovingFleets(void) {
	return _firstMovingFleet.next();
}

const BilistNode<Fleet> *GameState::getMovingFleets(void) const {
	return _firstMovingFleet.next();
}

void GameState::dump(void) const {
	fprintf(stdout, "=== Config ===\n");
	fprintf(stdout, "Version: %d\n", _gameConfig.version);
	fprintf(stdout, "Save game name: %s\n", _gameConfig.saveGameName);
	fprintf(stdout, "Stardate: %d\n", _gameConfig.stardate);
	fprintf(stdout, "End of turn summary: %d\n", _gameConfig.endOfTurnSummary);
	fprintf(stdout, "End of turn wait: %d\n", _gameConfig.endOfTurnWait);
	fprintf(stdout, "Random events: %d\n", _gameConfig.randomEvents);
	fprintf(stdout, "Enemy moves: %d\n", _gameConfig.enemyMoves);
	fprintf(stdout, "Expanding help: %d\n", _gameConfig.expandingHelp);
	fprintf(stdout, "Autoselect ships: %d\n", _gameConfig.autoSelectShips);
	fprintf(stdout, "Animations: %d\n", _gameConfig.animations);
	fprintf(stdout, "Auto select colony: %d\n", _gameConfig.autoSelectColony);
	fprintf(stdout, "Show relocation lines: %d\n", _gameConfig.showRelocationLines);
	fprintf(stdout, "Show GNN Report: %d\n", _gameConfig.showGNNReport);
	fprintf(stdout, "Auto delete trade good housing: %d\n", _gameConfig.autoDeleteTradeGoodHousing);
	fprintf(stdout, "Show only serious turn summary: %d\n", _gameConfig.showOnlySeriousTurnSummary);
	fprintf(stdout, "Ship initiative: %d\n", _gameConfig.shipInitiative);

	fprintf(stdout, "=== Galaxy ===\n");
	fprintf(stdout, "Size factor: %d\n", _galaxy.sizeFactor);
	fprintf(stdout, "width: %d\n", _galaxy.width);
	fprintf(stdout, "height: %d\n", _galaxy.height);

	fprintf(stdout, "\n=== Hero ===\n");
	for (int i = 0; i < LEADER_COUNT; i++) {
		fprintf(stdout, "Name: %s\n", _leaders[i].name);
		fprintf(stdout, "Title: %s\n", _leaders[i].title);
		fprintf(stdout, "Type: %d\n", _leaders[i].type);
		fprintf(stdout, "Experience: %d\n", _leaders[i].experience);
		fprintf(stdout, "Common skills: %lu\n", _leaders[i].commonSkills);
		fprintf(stdout, "Special skills: %lu\n", _leaders[i].specialSkills);
		for (int j = 0; j < MAX_LEADER_TECH_SKILLS; j++) {
			fprintf(stdout, "Tech: %d\n", _leaders[i].techs[j]);
		}
		fprintf(stdout, "Picture: %d\n", _leaders[i].picture);
		fprintf(stdout, "Skill value: %d\n", _leaders[i].skillValue);
		fprintf(stdout, "Level: %d\n", _leaders[i].level);
		fprintf(stdout, "Location: %d\n", _leaders[i].location);
		fprintf(stdout, "ETA: %d\n", _leaders[i].eta);
		fprintf(stdout, "Level up: %d\n", _leaders[i].displayLevelUp);
		fprintf(stdout, "Status: %d\n", _leaders[i].status);
		fprintf(stdout, "Player: %d\n", _leaders[i].playerIndex);
	}

	fprintf(stdout, "\n=== Player ===\n");
	for (int i = 0; i < PLAYER_COUNT; i++) {
		fprintf(stdout, "Name:\t%s\tRace:\t%s\n",
			_players[i].name, _players[i].race);
		fprintf(stdout, "Picture:\t\t%d\tColor:\t\t\t%d\tPersonality:\t\t%d\n",
			_players[i].picture, _players[i].color, _players[i].personality);
		fprintf(stdout, "Objective:\t\t%d\tTax rate:\t\t%d\tBC:\t\t\t%lu\n",
			_players[i].objective, _players[i].taxRate, _players[i].BC);
		fprintf(stdout, "Total freighters:\t%d\tUsed freighters:\t%d\tCommand points:\t\t%d\n",
			_players[i].totalFreighters, _players[i].surplusFreighters, _players[i].commandPoints);
		fprintf(stdout, "Total production:\t%d\tRP:\t\t\t%d\tFood:\t\t\t%d\n",
			_players[i].industryProduced, _players[i].researchProduced, _players[i].surplusFood);
		fprintf(stdout, "Yearly BC:\t\t%d\tResearch progress:\t%d\tResearch Area:\t\t%d\n",
			_players[i].surplusBC, _players[i].researchProduced, (int)_players[i].researchArea);
		fprintf(stdout, "Research Item:\t\t%d\n",
			_players[i].researchItem);

		fprintf(stdout, "--- Racepicks ---\n");
		fprintf(stdout, "Government:\t\t%d\tPopulation:\t\t%d\tFarming:\t\t%d\tScience:\t\t%d\n",
			_players[i].racePicks.government, _players[i].racePicks.population,
			_players[i].racePicks.farming, _players[i].racePicks.science);
		fprintf(stdout, "Money:\t\t\t%d\tShip defense:\t\t%d\tShip attack:\t\t%d\tGround combat:\t\t%d\n",
			_players[i].racePicks.money, _players[i].racePicks.shipDefense,
			_players[i].racePicks.shipAttack, _players[i].racePicks.groundCombat);
		fprintf(stdout, "Spying:\t\t\t%d\tLow G:\t\t\t%d\tHigh G:\t\t\t%d\tAquatic:\t\t%d\n",
			_players[i].racePicks.spying, _players[i].racePicks.lowG,
			_players[i].racePicks.highG, _players[i].racePicks.aquatic);
		fprintf(stdout, "Subterranian:\t\t%d\tLarge homeworld:\t%d\tRich/Poor homeworld:\t%d\tArtifacts homeworld:\t%d\n",
			_players[i].racePicks.subterranean, _players[i].racePicks.largeHomeworld,
			_players[i].racePicks.richHomeworld, _players[i].racePicks.artifactsHomeworld);
		fprintf(stdout, "Cybernetic:\t\t%d\tLithovore:\t\t%d\tRepulsive:\t\t%d\tCharismatic:\t\t%d\n",
			_players[i].racePicks.cybernetic, _players[i].racePicks.lithovore,
			_players[i].racePicks.repulsive, _players[i].racePicks.charismatic);
		fprintf(stdout, "Uncreative:\t\t%d\tCreative:\t\t%d\tTolerant:\t\t%d\tFantastic traders:\t%d\n",
			_players[i].racePicks.uncreative, _players[i].racePicks.creative,
			_players[i].racePicks.tolerant, _players[i].racePicks.fantasticTraders);
		fprintf(stdout, "Telepathic:\t\t%d\tLucky:\t\t\t%d\tOmniscience:\t\t%d\tStealthy ships:\t\t%d\n",
			_players[i].racePicks.telepathic, _players[i].racePicks.lucky,
			_players[i].racePicks.omniscience, _players[i].racePicks.stealthyShips);
		fprintf(stdout, "Transdimensional:\t%d\tWarlord:\t\t%d\n",
			_players[i].racePicks.transDimensional, _players[i].racePicks.warlord);
	}

	fprintf(stdout, "Number of stars: %d\n", _starSystemCount);
	for (auto star : _starSystems) {
		fprintf(stdout, "\nName:\t%s\n", star.name);
		fprintf(stdout, "Class:\t\t%x\t\tSize:\t\t%x\n", (unsigned)star.spectralClass, (unsigned)star.size);
		fprintf(stdout, "Position:\t%d,%d\tPrimary owner:\t%d\n", star.x, star.y, star.owner);
		fprintf(stdout, "Special:\t%d\t\tWormhole:\t%d\n", (int)star.special, star.wormhole);
	}
}

Fleet::Fleet(GameState *parent, unsigned flagship) : _parent(parent),
	_shipCount(0), _maxShips(8), _orbitedStar(-1), _destStar(-1) {

	Ship *fs;

	if (flagship >= _parent->_shipCount ||
		!_parent->_ships[flagship].isActive()) {
		throw std::invalid_argument("Invalid fleet flagship");
	}

	fs = _parent->_ships + flagship;

	switch (fs->status) {
	case ShipState::InOrbit:
		_orbitedStar = fs->getStarID();
		break;

	case ShipState::LeavingOrbit:
		_orbitedStar = _parent->findStar(fs->x, fs->y);
		// fall through

	case ShipState::InTransit:
		_destStar = fs->getStarID();
		break;

	default:
		throw std::logic_error("Fleet flagship has disallowed state");
	}

	_owner = fs->owner;
	_status = fs->status;
	_x = fs->x;
	_y = fs->y;
	// FIXME: recalculate properly and update ships
	_hasNavigator = fs->groupHasNavigator;
	_warpSpeed = fs->warpSpeed;
	_eta = fs->eta;

	_ships = new unsigned[_maxShips];
	_ships[_shipCount++] = flagship;
}

Fleet::Fleet(const Fleet &other) : _parent(other._parent), _ships(NULL),
	_shipCount(other._shipCount), _orbitedStar(other._orbitedStar),
	_destStar(other._destStar), _owner(other._owner),
	_status(other._status), _x(other._x), _y(other._y),
	_hasNavigator(other._hasNavigator), _warpSpeed(other._warpSpeed),
	_eta(other._eta) {

	_maxShips = _shipCount > 8 ? _shipCount : 8;
	_ships = new unsigned[_maxShips];
	memcpy(_ships, other._ships, _shipCount * sizeof(unsigned));
}

Fleet::~Fleet(void) {
	delete[] _ships;
}

void Fleet::addShip(unsigned ship_id) {
	Ship *s;
	int dest;

	if (ship_id >= _parent->_shipCount) {
		throw std::out_of_range("Invalid ship ID");
	}

	s = _parent->_ships + ship_id;

	if (s->getStarID() > _parent->_starSystemCount) {
		throw std::out_of_range("Ship has invalid star ID");
	}

	dest = s->status == ShipState::InOrbit ? -1 : s->getStarID();

	if (s->owner != _owner || s->status != _status || dest != _destStar ||
		s->x != _x || s->y != _y) {
		throw std::runtime_error("Ship state does not match fleet");
	}

	if (_shipCount >= _maxShips) {
		unsigned *tmp;
		size_t size = 2 * _maxShips;

		tmp = new unsigned[size];
		memcpy(tmp, _ships, _shipCount * sizeof(unsigned));
		delete[] _ships;
		_ships = tmp;
		_maxShips = size;
	}

	_ships[_shipCount++] = ship_id;
	// FIXME: update _hasNavigator, recalculate speed, eta and update ships
}

void Fleet::removeShip(size_t pos) {
	size_t i;

	if (pos >= _shipCount) {
		throw std::out_of_range("Invalid ship index");
	}

	for (i = pos; i < _shipCount - 1; i++) {
		_ships[i] = _ships[i + 1];
	}

	_shipCount--;
}

Ship *Fleet::getShip(size_t pos) {
	if (pos >= _shipCount) {
		throw std::out_of_range("Invalid ship index");
	}

	return _parent->_ships + _ships[pos];
}

const Ship *Fleet::getShip(size_t pos) const {
	if (pos >= _shipCount) {
		throw std::out_of_range("Invalid ship index");
	}

	return _parent->_ships + _ships[pos];
}

Star *Fleet::getOrbitedStar(void) {
	return _orbitedStar >= 0 ? _parent->_starSystems + _orbitedStar : NULL;
}

const Star *Fleet::getOrbitedStar(void) const {
	return _orbitedStar >= 0 ? _parent->_starSystems + _orbitedStar : NULL;
}

Star *Fleet::getDestStar(void) {
	return _destStar >= 0 ? _parent->_starSystems + _destStar : NULL;
}

const Star *Fleet::getDestStar(void) const {
	return _destStar >= 0 ? _parent->_starSystems + _destStar : NULL;
}

size_t Fleet::shipCount(void) const {
	return _shipCount;
}

uint8_t Fleet::getOwner(void) const {
	return _owner;
}

uint8_t Fleet::getStatus(void) const {
	return _status;
}

uint16_t Fleet::getX(void) const {
	return _x;
}

uint16_t Fleet::getY(void) const {
	return _y;
}
