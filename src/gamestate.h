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
const int PLAYER_RACE_SIZE 			= 0x10;

const int MAX_LEADER_TECH_SKILLS	= 3;
const int MAX_PLANETS_PER_SYSTEM	= 5;
const int MAX_PLAYERS				= 8;
const int MAX_TECHNOLOGIES 			= 0xcb;
const int MAX_STARS					= 72;
const int MAX_SETTLERS				= 25;

#define STAR_TYPE_COUNT 6

enum MultiplayerType {
	Single = 0,
	Hotseat = 1,
	Network = 2,
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

enum class SpecialType: uint8_t {
    None			= 0x00,
    SpecialDebris	= 0x02,
    PirateCache		= 0x03,
    GoldDeposites	= 0x04,
    GemDeposits		= 0x05,
    Natives			= 0x06,
    SplinterColony	= 0x07,
    Leader 			= 0x08,
    ArtifactsWorld	= 0x0A,
    Orion			= 0x0B
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

struct Galaxy {
	uint8_t sizeFactor;
	uint16_t width;
	uint16_t height;

	Galaxy(void);

	void load(ReadStream &stream);
};

struct Leader {
	char name[LEADER_NAME_SIZE];
	char title[LEADER_TITLE_SIZE];
	uint8_t type;
	uint16_t experience;
	// Bitmask for general skills
	uint64_t commonSkills;
	// Bitmask for including ship or colony skills
	uint64_t specialSkills;
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
	uint8_t picture;
	uint8_t color;
	uint8_t personality;
	uint8_t objective;
	uint16_t homePlayerId;
	uint16_t networkPlayerId;
	uint8_t playerDoneFlags;
	uint8_t researchBreakthrough;
	uint8_t taxRate;
	uint64_t BC;
	uint16_t totalFreighters;
	uint16_t surplusFreighters;
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
	ResearchArea researchArea;
	uint16_t researchItem;
	RacePicks racePicks;

	Player(void);

	void load(SeekableReadStream &stream);
};

struct Star {
	char name[STARS_NAME_SIZE];
	uint16_t x;
	uint16_t y;
	uint8_t size;
	/*
	* 0-7: player id
	* 0xFF: no owner
	*/
	uint8_t owner;
	uint8_t pictureType;
	uint8_t spectralClass;
	// Remembers the last selected planet for the system for each player
	uint8_t lastPlanetSelected[MAX_PLAYERS];
	// Precomputed bitfield that tells wether there is a black hole between two stars
	uint8_t blackHoleBlocks[(MAX_STARS + 7) / 8];
	SpecialType special;
	// 0-7F = system, FF = no wormhole
	uint8_t wormhole;
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
	uint16_t planetIndex[MAX_PLANETS_PER_SYSTEM];
	// Star index all the ships will be relocated TO
	uint16_t relocateShipTo[MAX_PLAYERS];
	// Usually this is -1, else the player to give the colonies to
	int8_t surrenderTo[MAX_PLAYERS];
	// Is this star in a nebula?
	uint8_t inNebula;
	// Has the ancient artifacts app been given out yet?
	uint8_t artifactsGaveApp;

	Star(void);

	void load(ReadStream &stream);
};

struct GameState  {
	struct GameConfig _gameConfig;
	struct Galaxy _galaxy;
	uint8_t _starSystemCount;
	Star _starSystems[MAX_STARS];
	struct Leader _leaders[LEADER_COUNT];
	struct Player _players[PLAYER_COUNT];

	void load(SeekableReadStream &stream);
	void load(const char *filename);
	void dump(void) const;
};

#endif
