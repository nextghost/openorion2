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

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include "utils.h"
#include "stream.h"
#include <vector>

const int SAVE_GAME_NAME_SIZE		= 37;
const int LEADER_COUNT 				= 67;
const int LEADER_RECORD_SIZE		= 0x3b;
const int LEADER_NAME_SIZE 			= 0x0f;
const int LEADER_TITLE_SIZE			= 0x14;

const int STARS_RECORD_SIZE 		= 113;
const int STARS_NAME_SIZE           = 15;

const int PLAYER_COUNT 				= 0x08;
const int PLAYER_NAME_SIZE 			= 0x14;
#define PLAYER_RACE_SIZE 15
#define RACE_COUNT 13

#define OBJECTIVE_HUMAN 100

#define SHIP_NAME_SIZE 16

const int MAX_LEADER_TECH_SKILLS	= 3;
const int MAX_PLAYERS				= 8;
const int MAX_TECHNOLOGIES 			= 0xcb;
const int MAX_SETTLERS				= 25;
#define MAX_STARS 72
#define MAX_ORBITS 5
#define MAX_COLONIES 250
#define MAX_PLANETS (MAX_STARS * MAX_ORBITS)
#define MAX_SHIP_SPECIALS 39
#define MAX_SHIP_WEAPONS 8
#define MAX_PLAYER_BLUEPRINTS 5
#define MAX_NEBULAS 4
#define MAX_SHIPS 500

#define MAX_POPULATION 42
#define MAX_RACES (MAX_PLAYERS+2)	// player races + androids + natives
#define MAX_BUILD_QUEUE 7
#define MAX_BUILDINGS 49
#define STAR_TYPE_COUNT 6
#define NEBULA_TYPE_COUNT 12
#define MAX_FLEET_OWNERS 15

#define MAX_SPIES 0x3f
#define SPY_MISSION_MASK 0xc0
#define SPY_MISSION_STEAL 0
#define SPY_MISSION_SABOTAGE 0x40
#define SPY_MISSION_HIDE 0x80

#define MAX_PLANET_BGS 3

enum MultiplayerType {
	Single = 0,
	Hotseat = 1,
	Network = 2,
};

enum PlanetType {
	ASTEROIDS = 1,
	GAS_GIANT = 2,
	HABITABLE = 3
};

enum PlanetSize {
	TINY_PLANET = 0,
	SMALL_PLANET = 1,
	MEDIUM_PLANET = 2,
	LARGE_PLANET = 3,
	HUGE_PLANET = 4
};

enum PlanetGravity {
	LOW_G = 0,
	NORMAL_G = 1,
	HEAVY_G = 2
};

enum PlanetClimate {
	TOXIC = 0,
	RADIATED = 1,
	BARREN = 2,
	DESERT = 3,
	TUNDRA = 4,
	OCEAN = 5,
	SWAMP = 6,
	ARID = 7,
	TERRAN = 8,
	GAIA = 9
};

enum PlanetMinerals {
	ULTRA_POOR = 0,
	POOR = 1,
	ABUNDANT = 2,
	RICH = 3,
	ULTRA_RICH = 4
};

enum ColonistRace {
	/* 0-7 = player race */
	ANDROID = 8,
	NATIVE = 9
};

enum ColonistFlags {
	WORKING = 0x1,
	PRISONER = 0x2
};

enum ColonistJob {
	FARMER = 0,
	WORKER = 1,
	SCIENTIST = 2
};

enum StarSize {
    Large 	= 0x00,
    Medium	= 0x01,
    Small 	= 0x02,
    Tiny 	= 0x03
};

enum SpectralClass {
    Blue 		= 0x00,
    White 		= 0x01,
    Yellow		= 0x02,
    Orange	 	= 0x03,
    Red 		= 0x04,
    Brown 		= 0x05,
    BlackHole	= 0x06
};

enum SpecialType {
	NO_SPECIAL = 0,
	BAD_SPECIAL1 = 1,
	SPACE_DEBRIS = 2,
	PIRATE_CACHE = 3,
	GOLD_DEPOSITS = 4,
	GEM_DEPOSITS = 5,
	NATIVES = 6,
	SPLINTER_COLONY = 7,
	LOST_HERO = 8,
	BAD_SPECIAL2 = 9,
	ANCIENT_ARTIFACTS = 10,
	ORION_SPECIAL = 11
};

enum class ResearchArea: uint8_t {
    None 			= 0x00,
    Construction 	= 0x04,
    ForceFields		= 0x07,
    Sociology 		= 0x0a,
    Biology 		= 0x12,
    Chemistry 		= 0x16,
    Computers 		= 0x1c,
    Power 			= 0x37,
    Physics			= 0x39
};

enum ShipState {
	InOrbit = 0,
	InTransit,
	LeavingOrbit,
	Unknown,	// FIXME
	InRefit,
	Destroyed,
	UnderConstruction
};

enum ShipType {
	COMBAT_SHIP = 0,
	COLONY_SHIP,
	TRANSPORT_SHIP,
	BAD_SHIP_TYPE,
	OUTPOST_SHIP
};

class Fleet;

struct GameConfig {
	uint32_t version;
	char saveGameName[SAVE_GAME_NAME_SIZE];
	unsigned int stardate;
	uint8_t multiplayer;
	uint8_t endOfTurnSummary;
	uint8_t endOfTurnWait;
	uint8_t randomEvents;
	uint8_t enemyMoves;
	uint8_t expandingHelp;
	uint8_t autoSelectShips;
	uint8_t animations;
	uint8_t autoSelectColony;
	uint8_t showRelocationLines;
	uint8_t showGNNReport;
	uint8_t autoDeleteTradeGoodHousing;
	uint8_t showOnlySeriousTurnSummary;
	uint8_t shipInitiative;

	GameConfig(void);

	void load(ReadStream &stream);
};

struct Nebula {
	uint16_t x, y;
	uint8_t type;

	Nebula(void);

	void load(ReadStream &stream);
};

struct Galaxy {
	uint8_t sizeFactor;
	uint16_t width;
	uint16_t height;
	Nebula nebulas[MAX_NEBULAS];
	uint8_t nebulaCount;

	Galaxy(void);

	void load(ReadStream &stream);
};

struct Colonist {
	uint8_t race;
	/* Prisoners stay loyal to the previous player until assimilation */
	uint8_t loyalty;
	uint8_t job;
	unsigned flags;

	Colonist(void);

	void load(ReadStream &stream);
};

struct Colony {
	uint8_t owner;
	int8_t unknown1;	// FIXME: analyze
	int16_t planet;
	int16_t unknown2;	// FIXME: analyze
	uint8_t is_outpost;
	int8_t morale;
	uint16_t pollution;
	uint8_t population;
	uint8_t colony_type;
	Colonist colonists[MAX_POPULATION];
	uint16_t race_population[MAX_RACES];
	int16_t pop_growth[MAX_RACES];

	/* Number of turns since the last change of colony owner. If the value
	 * grows to 255, it'll stay there until the next owner change or end
	 * of the game.
	 */
	uint8_t age;
	uint8_t food_per_farmer;	// in half-units
	uint8_t industry_per_worker;
	uint8_t research_per_scientist;
	int8_t max_farms;
	uint8_t max_population;

	/* Same value as planet climate except on a radiated planet with
	 * any of the planetary shields built. Planet climate stays radiated
	 * but colony climate changes to barren.
	 */
	uint8_t climate;
	uint16_t ground_strength;	// AI helper field
	uint16_t space_strength;	// AI helper field
	uint16_t total_food;
	uint16_t net_industry;
	uint16_t total_research;
	uint16_t total_revenue;
	uint8_t food_consumption;
	uint8_t industry_consumption;
	uint8_t research_consumption;
	uint16_t upkeep;
	int16_t food_imported;
	uint16_t industry_consumed;
	int16_t research_imported;
	int16_t budget_deficit;
	uint8_t recycled_industry;	// recyclotron

	// food/industry consumption in half-units by citizenship status
	uint8_t food_consumption_citizens;
	uint8_t food_consumption_aliens;
	uint8_t food_consumption_prisoners;
	uint8_t food_consumption_natives;
	uint8_t industry_consumption_citizens;
	uint8_t industry_consumption_androids;
	uint8_t industry_consumption_aliens;
	uint8_t industry_consumption_prisoners;

	// consumption in half-units per race (including prisoners)
	uint8_t food_consumption_races[MAX_PLAYERS];
	uint8_t industry_consumption_races[MAX_PLAYERS];

	uint8_t replicated_food;
	int16_t build_queue[MAX_BUILD_QUEUE];

	/* Set to the last completed item in the build queue when the queue
	 * becomes empty. The game will ask you to add new build orders when
	 * you open the colony view and finished_production != 0xffff.
	 * The value resets to 0xffff when that pop-up window shows up.
	 */
	int16_t finished_production;
	uint16_t build_progress;
	uint16_t tax_revenue;
	uint8_t autobuild;
	uint16_t unknown3;	// FIXME: analyze
	uint16_t bought_progress;	// build progress bought with cash

	/* Counter increases every turn if policy == assimilate, one prisoner
	 * gets assimilated when the counter reaches 240. Rate of increase
	 * depends on race traits and alien management center.
	 */
	uint8_t assimilation_progress;
	uint8_t prisoner_policy;	// 3 = assimilate, 0 = kill
	uint16_t soldiers;
	uint16_t tanks;

	/* Progress counters go up by 1 each turn when there's space for more
	 * units in the barracks. Another unit gets added when the counter
	 * reaches 5.
	 */
	uint8_t tank_progress;
	uint8_t soldier_progress;
	uint8_t buildings[MAX_BUILDINGS];
	uint16_t status;	// FIXME: analyze

	Colony(void);

	void load(ReadStream &stream);
};

struct Planet {
	int16_t colony;
	uint8_t star;
	uint8_t orbit;
	uint8_t type;
	uint8_t size;
	uint8_t gravity;
	uint8_t unknown1;
	uint8_t climate;
	uint8_t bg;
	uint8_t minerals;
	uint8_t foodbase;
	uint8_t terraforms;
	uint8_t unknown2;
	uint8_t max_pop;
	uint8_t special;
	uint8_t flags;

	Planet(void);

	void load(ReadStream &stream);
};

struct Leader {
	char name[LEADER_NAME_SIZE];
	char title[LEADER_TITLE_SIZE];
	uint8_t type;
	uint16_t experience;
	// Bitmask for general skills
	uint32_t commonSkills;
	// Bitmask for including ship or colony skills
	uint32_t specialSkills;
	uint8_t techs[MAX_LEADER_TECH_SKILLS];
	uint8_t picture;
	uint16_t skillValue;
	uint8_t level;
	int16_t location;
	uint8_t eta;
	// if true show level up popup for the leader
	uint8_t displayLevelUp;
	uint8_t status;
	uint8_t playerIndex;

	Leader(void);

	void load(ReadStream &stream);
};

struct ShipWeapon {
	uint8_t type;	// FIXME: uint16_t?
	uint8_t maxCount, workingCount;
	uint8_t arc, beamMods, missileMods, ammo;

	void load(ReadStream &stream);
};

struct ShipDesign {
	char name[SHIP_NAME_SIZE];
	uint8_t size, type;
	uint8_t shield, drive, speed, computer, armor;
	uint8_t specials[(MAX_SHIP_SPECIALS + 7) / 8];
	ShipWeapon weapons[MAX_SHIP_WEAPONS];
	uint8_t picture, builder;
	uint16_t cost;
	uint8_t combatSpeed;
	uint16_t buildDate;

	ShipDesign(void);

	void load(ReadStream &stream);
};

struct RacePicks {
	uint8_t government;
	int8_t population;
	int8_t farming;
	int8_t industry;
	int8_t science;
	int8_t money;
	int8_t shipDefense;
	int8_t shipAttack;
	int8_t groundCombat;
	int8_t spying;
	uint8_t lowG;
	uint8_t highG;
	uint8_t aquatic;
	uint8_t subterranean;
	uint8_t largeHomeworld;
	int8_t richHomeworld;
	uint8_t artifactsHomeworld;
	uint8_t cybernetic;
	uint8_t lithovore;
	uint8_t repulsive;
	uint8_t charismatic;
	uint8_t uncreative;
	uint8_t creative;
	uint8_t tolerant;
	uint8_t fantasticTraders;
	uint8_t telepathic;
	uint8_t lucky;
	uint8_t omniscience;
	uint8_t stealthyShips;
	uint8_t transDimensional;
	uint8_t warlord;
	uint8_t poorHomeworld;

	void load(ReadStream &stream);
};

// Maybe we have padding after job field to fill until 32bits
struct SettlerInfo {
	unsigned sourceColony;
	unsigned destinationPlanet;
	unsigned player;
	unsigned eta;
	unsigned job;

	void load(ReadStream &stream);
};

struct Player {
	char name[PLAYER_NAME_SIZE];
	char race[PLAYER_RACE_SIZE];
	uint8_t eliminated;
	uint8_t picture;
	uint8_t color;
	uint8_t personality;
	uint8_t objective;
	uint16_t homePlayerId;
	uint16_t networkPlayerId;
	uint8_t playerDoneFlags;
	uint8_t researchBreakthrough;
	uint8_t taxRate;
	int32_t BC;
	uint16_t totalFreighters;
	int16_t surplusFreighters;
	uint16_t commandPoints;
	int16_t usedCommandPoints;
	// Sum of the following 3 should be <= totalFreighters * FREIGHTER_CAPACITY
	// FIXME: ^^^ ???
	uint16_t foodFreighted;
	uint16_t settlersFreighted;
	SettlerInfo settlers[MAX_SETTLERS];
	uint16_t totalPop;
	int16_t foodProduced;
	int16_t industryProduced;
	int16_t researchProduced;
	int16_t bcProduced;
	int16_t surplusFood;
	int16_t surplusBC;
	int32_t totalMaintenance;
	uint32_t researchProgress;
	ResearchArea researchArea;
	uint16_t researchItem;
	ShipDesign blueprints[MAX_PLAYER_BLUEPRINTS], selectedBlueprint;
	RacePicks racePicks;
	uint8_t spies[MAX_PLAYERS];

	Player(void);

	void load(SeekableReadStream &stream);
};

class Star {
private:
	BilistNode<Fleet> _firstOrbitingFleet, _lastOrbitingFleet;
	BilistNode<Fleet> _firstLeavingFleet, _lastLeavingFleet;

	// Do NOT implement
	Star(const Star &other);
	const Star &operator=(const Star &other);

public:
	char name[STARS_NAME_SIZE];
	uint16_t x;
	uint16_t y;
	uint8_t size;
	int8_t owner;
	uint8_t pictureType;
	uint8_t spectralClass;
	// Remembers the last selected planet for the system for each player
	uint8_t lastPlanetSelected[MAX_PLAYERS];
	// Precomputed bitfield that tells wether there is a black hole between two stars
	uint8_t blackHoleBlocks[(MAX_STARS + 7) / 8];
	uint8_t special;
	// 0-7F = system, FF = no wormhole
	int8_t wormhole;
	// Bitmask that tells if a uses is blockaded
	uint8_t blockaded;
	/*
	 * Bitmask indexed by player.
	 * That tells whether given player blockades index player.
	 * Used primarily for diplomacy so AI can tell which players are blockading them at a star
	 * TEST_BIT(blockaded_by[i], j) returns true or false, being i and j player indexes
	 */
	uint8_t blockadedBy[MAX_PLAYERS];
	// Bitmask that tells whether given player has ever visited
	uint8_t visited;
	// Bitmask that tells whether each player first visited on previous turn and we should get a report
	uint8_t justVisited;
	// Bitmask that tells whether each player requested ignore colony ships
	// Reset each time a new colony ship enters a star
	uint8_t ignoreColonyShips;
	// Bitmask that tells whether each player requested ignore combat ships
	// Reset each time a new combat ship enters a star
	uint8_t ignoreCombatShips;
	// Bitmask: 0-7 or -1
	int8_t colonizePlayer;
	// Bitmask that tells whether each player has a colony
	uint8_t hasColony;
	// Bitmask that tells whether each player has a warp field interdictor
	// 0 = none, 1-8 = owner player 0-7
	uint8_t hasWarpFieldInterdictor;
	uint8_t nextWFIInList;
	// Bitmask that tells whether each player has a tachyon
	uint8_t hasTachyon;
	// Bitmask that tells whether each player has a subspace
	uint8_t hasSubspace;
	// Bitmask that tells whether each player has a stargate
	uint8_t hasStargate;
	// Bitmask that tells whether each player has a jumpgate
	uint8_t hasJumpgate;
	// Bitmask that tells whether each player has an artemis net
	uint8_t hasArtemisNet;
	// Bitmask that tells whether each player has a dimensional portal
	uint8_t hasDimensionalPortal;
	// Bitmask that tells whether star is stagepoint for each AI
	uint8_t isStagepoint;
	// Bitmask that tells whether an officer is in the system roster
	uint8_t officerIndex[MAX_PLAYERS];
	int16_t planetIndex[MAX_ORBITS];
	// Star index all the ships will be relocated TO
	uint16_t relocateShipTo[MAX_PLAYERS];
	// Usually this is -1, else the player to give the colonies to
	int8_t surrenderTo[MAX_PLAYERS];
	// Is this star in a nebula?
	uint8_t inNebula;
	// Has the ancient artifacts app been given out yet?
	uint8_t artifactsGaveApp;

	Star(void);
	~Star(void);

	void load(ReadStream &stream);

	void addFleet(Fleet *f);
	BilistNode<Fleet> *getOrbitingFleets(void);
	BilistNode<Fleet> *getLeavingFleets(void);
	const BilistNode<Fleet> *getOrbitingFleets(void) const;
	const BilistNode<Fleet> *getLeavingFleets(void) const;
};

struct Ship {
	ShipDesign design;
	uint8_t owner, status;
	uint16_t star, x, y;
	uint8_t groupHasNavigator;
	uint8_t warpSpeed, eta;
	uint8_t shieldDamage, driveDamage;	// percent
	uint8_t computerDamage, crewLevel;
	uint16_t crewExp;
	int16_t officer;
	uint8_t damagedSpecials[(MAX_SHIP_SPECIALS + 7) / 8];
	uint16_t armorDamage, structureDamage;
	uint8_t mission;	// AI field
	uint8_t justBuilt;

	Ship(void);

	bool operator<(const Ship &other) const;
	bool operator<=(const Ship &other) const;
	bool operator>(const Ship &other) const;
	bool operator>=(const Ship &other) const;

	void load(ReadStream &stream);

	// _starSystemCount is the special ID of Antaran homeworld
	unsigned getStarID(void) const;
	int isActive(void) const;
	int exists(void) const;
};

class GameState {
private:
	BilistNode<Fleet> _firstMovingFleet, _lastMovingFleet;

	// Do NOT implement
	GameState(const GameState &other);
	const GameState &operator=(const GameState &other);

protected:
	Fleet *findFleet(unsigned owner, unsigned status, unsigned x,
		unsigned y, unsigned star);
	void createFleets(void);

	void addFleet(Fleet *flt);
	void removeFleet(Fleet *flt);

public:
	struct GameConfig _gameConfig;
	struct Galaxy _galaxy;
	uint16_t _starSystemCount;
	Star _starSystems[MAX_STARS];
	uint16_t _colonyCount;
	Colony _colonies[MAX_COLONIES];
	uint16_t _planetCount;
	Planet _planets[MAX_PLANETS];
	struct Leader _leaders[LEADER_COUNT];
	uint16_t _playerCount;
	struct Player _players[PLAYER_COUNT];
	uint16_t _shipCount;
	Ship _ships[MAX_SHIPS];

	GameState(void);
	~GameState(void);

	void load(SeekableReadStream &stream);
	void load(const char *filename);
	void validate(void) const;
	void dump(void) const;

	unsigned findStar(int x, int y) const;
	BilistNode<Fleet> *getMovingFleets(void);
	const BilistNode<Fleet> *getMovingFleets(void) const;
};

class Fleet : public Recyclable {
private:
	GameState *_parent;
	unsigned *_ships;
	size_t _shipCount, _combatCount, _maxShips;
	int _orbitedStar, _destStar;
	uint8_t _owner, _status;
	uint16_t _x, _y;
	uint8_t _hasNavigator;
	uint8_t _warpSpeed, _eta;

	// Do NOT implement
	const Fleet &operator=(const Fleet &other);

public:
	Fleet(GameState *parent, unsigned flagship);
	Fleet(const Fleet &other);
	~Fleet(void);

	// Adding ships is allowed only during fleet creation. If you need
	// to add more ships to existing fleet, copy and discard it instead.
	// Removing ships is allowed at any time.
	void addShip(unsigned ship_id);
	void removeShip(size_t pos);

	Ship *getShip(size_t pos);
	const Ship *getShip(size_t pos) const;
	Star *getOrbitedStar(void);
	const Star *getOrbitedStar(void) const;
	Star *getDestStar(void);
	const Star *getDestStar(void) const;

	size_t shipCount(void) const;
	size_t combatCount(void) const;
	size_t supportCount(void) const;
	uint8_t getOwner(void) const;
	uint8_t getStatus(void) const;
	uint16_t getX(void) const;
	uint16_t getY(void) const;
};

#endif
