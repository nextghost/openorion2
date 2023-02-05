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

#define SAVE_GAME_NAME_SIZE 37
#define LEADER_COUNT 67
#define LEADER_NAME_SIZE 0x0f
#define LEADER_TITLE_SIZE 0x14
#define LEADER_TYPE_CAPTAIN 0
#define LEADER_TYPE_ADMIN 1
#define MAX_LEADER_LEVELS 5
#define MAX_SKILL_TYPES 3
#define MAX_COMMON_SKILLS 10
#define MAX_CAPTAIN_SKILLS 8
#define MAX_ADMIN_SKILLS 9
#define MAX_SKILLS (MAX_COMMON_SKILLS + MAX_CAPTAIN_SKILLS + MAX_ADMIN_SKILLS)
#define COMMON_SKILLS_TYPE 0x0
#define CAPTAIN_SKILLS_TYPE 0x10
#define ADMIN_SKILLS_TYPE 0x20
#define SKILLTYPE_MASK 0x30
#define SKILLCODE_MASK 0x0f
#define SKILLTYPE(id) (((id) & SKILLTYPE_MASK) >> 4)

#define STARS_NAME_SIZE 15

#define PLAYER_NAME_SIZE 0x14
#define PLAYER_RACE_SIZE 15
#define RACE_COUNT 13
#define GRAVITY_LEVEL_COUNT 3

#define OBJECTIVE_HUMAN 100

#define SHIP_NAME_SIZE 16

#define MAX_LEADER_TECH_SKILLS 3
#define MAX_PLAYERS 8
#define MAX_RESEARCH_AREAS 8
#define MAX_RESEARCH_TOPICS 83
#define MAX_APPLIED_TECHS 204
#define MAX_TECHNOLOGIES (MAX_APPLIED_TECHS + MAX_RESEARCH_AREAS)
#define MAX_SETTLERS 25
#define MAX_HISTORY_LENGTH 350
#define MAX_STARS 72
#define MAX_ORBITS 5
#define MAX_COLONIES 250
#define MAX_PLANETS (MAX_STARS * MAX_ORBITS)
#define MAX_SHIP_SPECIALS 40
#define MAX_SHIP_WEAPONS 8
#define MAX_PLAYER_BLUEPRINTS 5
#define MAX_NEBULAS 4
#define MAX_SHIPS 500

#define MAX_POPULATION 42
#define MAX_RACES (MAX_PLAYERS+2)	// player races + androids + natives
#define MAX_BUILD_QUEUE 7
#define MAX_BUILDINGS 49
#define STAR_TYPE_COUNT 6
#define GALAXY_ZOOM_LEVELS 4
#define GALAXY_STAR_SIZES 6
#define NEBULA_TYPE_COUNT 12
#define MAX_FLEET_OWNERS 15
#define NPC_FLEET_OWNERS (MAX_FLEET_OWNERS - MAX_PLAYERS)

#define MAX_SPIES 0x3f
#define SPY_MISSION_MASK 0xc0
#define SPY_MISSION_STEAL 0
#define SPY_MISSION_SABOTAGE 0x40
#define SPY_MISSION_HIDE 0x80

#define PLANET_CLIMATE_COUNT 10
#define PLANET_SIZE_COUNT 5
#define PLANET_MINERALS_COUNT 5
#define MAX_PLANET_BGS 3

#define MAX_SHIP_SPRITES 49
#define SHIPSPRITE_ANTARAN 0
#define SHIPSPRITE_GUARDIAN 7
#define SHIPSPRITE_MONSTER 8
#define SHIPSPRITE_MINIMONSTER 20
#define MAX_SHIPTYPES_ANTARAN 5
#define MAX_SHIPTYPES_MONSTER 5
#define MAX_SHIP_TYPES 5
#define MAX_COMBAT_SHIP_CLASSES 6

#define MAX_SHIP_CREW_LEVELS 5
#define MAX_SHIP_WEAPON_TYPES 46
#define MAX_SHIP_WEAPON_ARCS 5
#define MAX_SHIP_SHIELD_TYPES 6
#define MAX_SHIP_DRIVE_TYPES 7
#define MAX_SHIP_COMPUTER_TYPES 6
#define MAX_SHIP_ARMOR_TYPES 7

#define BUILDING_BIOSPHERES 15

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
	Large = 0,
	Medium = 1,
	Small = 2,
	Tiny = 3
};

enum SpectralClass {
	Blue = 0,
	White = 1,
	Yellow = 2,
	Orange = 3,
	Red = 4,
	Brown = 5,
	BlackHole = 6
};

enum StarKnowledge {
	STAR_UNEXPLORED = 0,
	STAR_NAME_ONLY,	// uncharted star owned by contacted player, name
			// is visible but no planet information available
	STAR_CHARTED,	// partial knowledge from galactic lore skill, etc.
	STAR_VISITED	// full star system knowledge
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

enum ResearchStatus {
	RSTATE_DISABLED = 0,	// cannot be researched, only traded/stolen
	RSTATE_RESEARCHABLE = 1,// can be researched later
	RSTATE_READY = 2,	// can be researched now (researchTopics only)
	RSTATE_KNOWN = 3	// has been researched/traded/stolen
};

enum ResearchArea {
	RESEARCH_BIOLOGY = 0,
	RESEARCH_POWER,
	RESEARCH_PHYSICS,
	RESEARCH_CONSTRUCTION,
	RESEARCH_FIELDS,
	RESEARCH_CHEMISTRY,
	RESEARCH_COMPUTERS,
	RESEARCH_SOCIOLOGY
};

enum ResearchTopic {
	TOPIC_STARTING_TECH = 0,
	TOPIC_ADVANCED_BIOLOGY,
	TOPIC_ADVANCED_CHEMISTRY,
	TOPIC_ADVANCED_CONSTRUCTION,
	TOPIC_ADVANCED_ENGINEERING,
	TOPIC_ADVANCED_FUSION,
	TOPIC_ADVANCED_GOVERNMENTS,
	TOPIC_ADVANCED_MAGNETISM,
	TOPIC_ADVANCED_MANUFACTURING,
	TOPIC_ADVANCED_METALLURGY,
	TOPIC_MILITARY_TACTICS,
	TOPIC_ADVANCED_ROBOTICS,
	TOPIC_TEACHING_METHODS,
	TOPIC_ANTIMATTER_FISSION,
	TOPIC_ARTIFICIAL_CONSCIOUSNESS,
	TOPIC_ARTIFICIAL_INTELLIGENCE,
	TOPIC_ARTIFICIAL_GRAVITY,
	TOPIC_ARTIFICIAL_LIFE,
	TOPIC_ASTRO_BIOLOGY,
	TOPIC_ASTRO_CONSTRUCTION,
	TOPIC_ASTRO_ENGINEERING,
	TOPIC_CAPSULE_CONSTRUCTION,
	TOPIC_CHEMISTRY,
	TOPIC_COLD_FUSION,
	TOPIC_CYBERTECHNICS,
	TOPIC_CYBERTRONICS,
	TOPIC_DISTORTION_FIELDS,
	TOPIC_ELECTROMAGNETIC_REFRACTION,
	TOPIC_ELECTRONICS,
	TOPIC_ENGINEERING,
	TOPIC_EVOLUTIONARY_GENETICS,
	TOPIC_FUSION_PHYSICS,
	TOPIC_GALACTIC_ECONOMICS,
	TOPIC_GALACTIC_NETWORKING,
	TOPIC_GENETIC_ENGINEERING,
	TOPIC_GENETIC_MUTATIONS,
	TOPIC_GRAVITIC_FIELDS,
	TOPIC_HIGH_ENERGY_DISTRIBUTION,
	TOPIC_HYPER_DIMENSIONAL_FISSION,
	TOPIC_HYPER_DIMENSIONAL_PHYSICS,
	TOPIC_INTERPHASED_FISSION,
	TOPIC_ION_FISSION,
	TOPIC_SUPERSCALAR_CONSTRUCTION,
	TOPIC_MACRO_ECONOMICS,
	TOPIC_MACRO_GENETICS,
	TOPIC_MAGNETO_GRAVITICS,
	TOPIC_MATTER_ENERGY_CONVERSION,
	TOPIC_MOLECULAR_COMPRESSION,
	TOPIC_MOLECULAR_CONTROL,
	TOPIC_MOLECULATRONICS,
	TOPIC_MOLECULAR_MANIPULATION,
	TOPIC_MULTIDIMENSIONAL_PHYSICS,
	TOPIC_MULTIPHASED_PHYSICS,
	TOPIC_NANO_TECHNOLOGY,
	TOPIC_NEUTRINO_PHYSICS,
	TOPIC_NUCLEAR_FISSION,
	TOPIC_OPTRONICS,
	TOPIC_PHYSICS,
	TOPIC_PLANETOID_CONSTRUCTION,
	TOPIC_PLASMA_PHYSICS,
	TOPIC_POSITRONICS,
	TOPIC_QUANTUM_FIELDS,
	TOPIC_ROBOTICS,
	TOPIC_SERVO_MECHANICS,
	TOPIC_SUBSPACE_FIELDS,
	TOPIC_SUBSPACE_PHYSICS,
	TOPIC_TACHYON_PHYSICS,
	TOPIC_TECTONIC_ENGINEERING,
	TOPIC_TEMPORAL_FIELDS,
	TOPIC_TEMPORAL_PHYSICS,
	TOPIC_TRANS_GENETICS,
	TOPIC_TRANSWARP_FIELDS,
	TOPIC_WARP_FIELDS,
	TOPIC_XENO_RELATIONS,
	TOPIC_XENON_TECHNOLOGY,
	TOPIC_HYPER_BIOLOGY,
	TOPIC_HYPER_POWER,
	TOPIC_HYPER_PHYSICS,
	TOPIC_HYPER_CONSTRUCTION,
	TOPIC_HYPER_FIELDS,
	TOPIC_HYPER_CHEMISTRY,
	TOPIC_HYPER_COMPUTERS,
	TOPIC_HYPER_SOCIOLOGY
};

enum Technology {
	TECH_NONE = 0,
	TECH_ACHILLES_TARGETING_UNIT,
	TECH_ADAMANTIUM_ARMOR,
	TECH_ADVANCED_CITY_PLANNING,
	TECH_ADVANCED_DAMAGE_CONTROL,
	TECH_ALIEN_MANAGEMENT_CENTER,
	TECH_ANDROID_FARMERS,
	TECH_ANDROID_SCIENTISTS,
	TECH_ANDROID_WORKERS,
	TECH_ANTIGRAV_HARNESS,
	TECH_ANTIMATTER_BOMB,
	TECH_ANTIMATTER_DRIVE,
	TECH_ANTIMATTER_TORPEDOES,
	TECH_ANTIMISSILE_ROCKETS,
	TECH_ARMOR_BARRACKS,
	TECH_ARTEMIS_SYSTEM_NET,
	TECH_PLANET_CONSTRUCTION,
	TECH_ASSAULT_SHUTTLES,
	TECH_ASTRO_UNIVERSITY,
	TECH_ATMOSPHERIC_RENEWER,
	TECH_AUGMENTED_ENGINES,
	TECH_AUTOLAB,
	TECH_AUTOMATED_FACTORIES,
	TECH_AUTOMATED_REPAIR_UNIT,
	TECH_BATTLEOIDS,
	TECH_BATTLE_PODS,
	TECH_BATTLE_SCANNER,
	TECH_BATTLESTATION,
	TECH_BIOTERMINATOR,
	TECH_BIOMORPHIC_FUNGI,
	TECH_BLACK_HOLE_GENERATOR,
	TECH_BOMBER_BAYS,
	TECH_CAPITOL,
	TECH_CLASS_I_SHIELD,
	TECH_CLASS_III_SHIELD,
	TECH_CLASS_V_SHIELD,
	TECH_CLASS_VII_SHIELD,
	TECH_CLASS_X_SHIELD,
	TECH_CLOAKING_DEVICE,
	TECH_CLONING_CENTER,
	TECH_COLONY_BASE,
	TECH_COLONY_SHIP,
	TECH_CONFEDERATION,
	TECH_CYBERSECURITY_LINK,
	TECH_CYBERTRONIC_COMPUTER,
	TECH_DAMPER_FIELD,
	TECH_DAUNTLESS_GUIDANCE_SYSTEM,
	TECH_DEATH_RAY,
	TECH_DEATH_SPORES,
	TECH_DEEP_CORE_MINING,
	TECH_CORE_WASTE_DUMPS,
	TECH_DEUTERIUM_FUEL_CELLS,
	TECH_DIMENSIONAL_PORTAL,
	TECH_DISPLACEMENT_DEVICE,
	TECH_DISRUPTER_CANNON,
	TECH_DOOM_STAR_CONSTRUCTION,
	TECH_REINFORCED_HULL,
	TECH_ECM_JAMMER,
	TECH_ELECTRONIC_COMPUTER,
	TECH_EMISSIONS_GUIDANCE_SYSTEM,
	TECH_ENERGY_ABSORBER,
	TECH_BIOSPHERES,
	TECH_EVOLUTIONARY_MUTATION,
	TECH_EXTENDED_FUEL_TANKS,
	TECH_FAST_MISSILE_RACKS,
	TECH_FEDERATION,
	TECH_FIGHTER_BAYS,
	TECH_FIGHTER_GARRISON,
	TECH_FOOD_REPLICATORS,
	TECH_FREIGHTERS,
	TECH_FUSION_BEAM,
	TECH_FUSION_BOMB,
	TECH_FUSION_DRIVE,
	TECH_FUSION_RIFLE,
	TECH_GAIA_TRANSFORMATION,
	TECH_GALACTIC_CURRENCY_EXCHANGE,
	TECH_GALACTIC_CYBERNET,
	TECH_GALACTIC_UNIFICATION,
	TECH_GAUSS_CANNON,
	TECH_GRAVITON_BEAM,
	TECH_GYRO_DESTABILIZER,
	TECH_HARD_SHIELDS,
	TECH_HEAVY_ARMOR,
	TECH_HEAVY_FIGHTER_BAYS,
	TECH_HEIGHTENED_INTELLIGENCE,
	TECH_HIGH_ENERGY_FOCUS,
	TECH_HOLO_SIMULATOR,
	TECH_HYDROPONIC_FARM,
	TECH_HYPER_DRIVE,
	TECH_MEGAFLUXERS,
	TECH_HYPERX_CAPACITORS,
	TECH_HYPERSPACE_COMMUNICATIONS,
	TECH_IMPERIUM,
	TECH_INERTIAL_NULLIFIER,
	TECH_INERTIAL_STABILIZER,
	TECH_INTERPHASED_DRIVE,
	TECH_ION_DRIVE,
	TECH_ION_PULSE_CANNON,
	TECH_IRIDIUM_FUEL_CELLS,
	TECH_JUMP_GATE,
	TECH_LASER_CANNON,
	TECH_LASER_RIFLE,
	TECH_LIGHTNING_FIELD,
	TECH_MARINE_BARRACKS,
	TECH_MASS_DRIVER,
	TECH_MAULER_DEVICE,
	TECH_MERCULITE_MISSILE,
	TECH_MICROBIOTICS,
	TECH_MICROLITE_CONSTRUCTION,
	TECH_OUTPOST_SHIP,
	TECH_MOLECULARTRONIC_COMPUTER,
	TECH_MULTIWAVE_ECM_JAMMER,
	TECH_MULTIPHASED_SHIELDS,
	TECH_NANO_DISASSEMBLERS,
	TECH_NEURAL_SCANNER,
	TECH_NEUTRON_BLASTER,
	TECH_NEUTRON_SCANNER,
	TECH_NEUTRONIUM_ARMOR,
	TECH_NEUTRONIUM_BOMB,
	TECH_NUCLEAR_BOMB,
	TECH_NUCLEAR_DRIVE,
	TECH_NUCLEAR_MISSILE,
	TECH_OPTRONIC_COMPUTER,
	TECH_PARTICLE_BEAM,
	TECH_PERSONAL_SHIELD,
	TECH_PHASE_SHIFTER,
	TECH_PHASING_CLOAK,
	TECH_PHASOR,
	TECH_PHASOR_RIFLE,
	TECH_PLANETARY_BARRIER_SHIELD,
	TECH_PLANETARY_FLUX_SHIELD,
	TECH_PLANETARY_GRAVITY_GENERATOR,
	TECH_PLANETARY_MISSILE_BASE,
	TECH_GROUND_BATTERIES,
	TECH_PLANETARY_RADIATION_SHIELD,
	TECH_PLANETARY_STOCK_EXCHANGE,
	TECH_PLANETARY_SUPERCOMPUTER,
	TECH_PLASMA_CANNON,
	TECH_PLASMA_RIFLE,
	TECH_PLASMA_TORPEDOES,
	TECH_PLASMA_WEB,
	TECH_PLEASURE_DOME,
	TECH_POLLUTION_PROCESSOR,
	TECH_POSITRONIC_COMPUTER,
	TECH_POWERED_ARMOR,
	TECH_PULSE_RIFLE,
	TECH_PROTON_TORPEDOES,
	TECH_PSIONICS,
	TECH_PULSAR,
	TECH_PULSON_MISSILE,
	TECH_QUANTUM_DETONATOR,
	TECH_RANGEMASTER_UNIT,
	TECH_RECYCLOTRON,
	TECH_REFLECTION_FIELD,
	TECH_ROBOTIC_FACTORY,
	TECH_RESEARCH_LABORATORY,
	TECH_ROBOMINERS,
	TECH_SPACE_SCANNER,
	TECH_SCOUT_LAB,
	TECH_SECURITY_STATIONS,
	TECH_SENSORS,
	TECH_SHIELD_CAPACITORS,
	TECH_SOIL_ENRICHMENT,
	TECH_SPACE_ACADEMY,
	TECH_SPACEPORT,
	TECH_SPATIAL_COMPRESSOR,
	TECH_SPY_NETWORK,
	TECH_STANDARD_FUEL_CELLS,
	TECH_STAR_BASE,
	TECH_STAR_FORTRESS,
	TECH_STAR_GATE,
	TECH_STASIS_FIELD,
	TECH_STEALTH_FIELD,
	TECH_STEALTH_SUIT,
	TECH_STELLAR_CONVERTER,
	TECH_STRUCTURAL_ANALYZER,
	TECH_SUBSPACE_COMMUNICATIONS,
	TECH_SUBSPACE_TELEPORTER,
	TECH_SUBTERRANEAN_FARMS,
	TECH_SURVIVAL_PODS,
	TECH_TACHYON_COMMUNICATIONS,
	TECH_TACHYON_SCANNER,
	TECH_TELEPATHIC_TRAINING,
	TECH_TERRAFORMING,
	TECH_THORIUM_FUEL_CELLS,
	TECH_TIME_WARP_FACILITATOR,
	TECH_TITAN_CONSTRUCTION,
	TECH_TITANIUM_ARMOR,
	TECH_TRACTOR_BEAM,
	TECH_TRANSPORT,
	TECH_TRANSPORTERS,
	TECH_TRITANIUM_ARMOR,
	TECH_TROOP_PODS,
	TECH_UNIVERSAL_ANTIDOTE,
	TECH_URRIDIUM_FUEL_CELLS,
	TECH_VIRTUAL_REALITY_NETWORK,
	TECH_WARP_DISSIPATER,
	TECH_WARP_INTERDICTOR,
	TECH_WEATHER_CONTROL_SYSTEM,
	TECH_WIDE_AREA_JAMMER,
	TECH_XENO_PSYCHOLOGY,
	TECH_XENTRONIUM_ARMOR,
	TECH_ZEON_MISSILE,
	TECH_ZORTRIUM_ARMOR,
	TECH_HYPER_BIOLOGY,
	TECH_HYPER_POWER,
	TECH_HYPER_PHYSICS,
	TECH_HYPER_CONSTRUCTION,
	TECH_HYPER_FIELDS,
	TECH_HYPER_CHEMISTRY,
	TECH_HYPER_COMPUTERS,
	TECH_HYPER_SOCIOLOGY
};

enum ForeignPolicy {
	DIPLO_NONE = 0,
	DIPLO_NON_AGGRESSION = 1,
	DIPLO_ALLIANCE = 2,
	DIPLO_PEACE = 3,
	DIPLO_LIMITED_WAR = 4,
	DIPLO_WAR = 5	// also values >5 => war
};

enum LeaderSkills {
	SKILL_ASSASSIN = COMMON_SKILLS_TYPE,
	SKILL_COMMANDO,
	SKILL_DIPLOMAT,
	SKILL_FAMOUS,
	SKILL_MEGAWEALTH,
	SKILL_OPERATIONS,
	SKILL_RESEARCHER,
	SKILL_SPYMASTER,
	SKILL_TELEPATH,
	SKILL_TRADER,

	SKILL_ENGINEER = CAPTAIN_SKILLS_TYPE,
	SKILL_FIGHTER_PILOT,
	SKILL_GALACTIC_LORE,
	SKILL_HELMSMAN,
	SKILL_NAVIGATOR,
	SKILL_ORDNANCE,
	SKILL_SECURITY,
	SKILL_WEAPONRY,

	SKILL_ENVIRONMENTALIST = ADMIN_SKILLS_TYPE,
	SKILL_FARMING_LEADER,
	SKILL_FINANCIAL_LEADER,
	SKILL_INSTRUCTOR,
	SKILL_LABOR_LEADER,
	SKILL_MEDICINE,
	SKILL_SCIENCE_LEADER,
	SKILL_SPIRITUAL_LEADER,
	SKILL_TACTICS
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

enum CombatShipClass {
	SHIP_FRIGATE = 0,
	SHIP_DESTROYER,
	SHIP_CRUISER,
	SHIP_BATTLESHIP,
	SHIP_TITAN,
	SHIP_DOOMSTAR
};

enum WeaponArc {
	ARC_FWD = 0x1,
	ARC_FWD_EXT = 0x2,
	ARC_BACK_EXT = 0x4,
	ARC_BACK = 0x8,
	ARC_MONSTER_360 = 0xf,
	ARC_360 = 0x10
};

enum SpecialDevices {
	SPEC_ACHILLES_TARGETING_UNIT = 1,
	SPEC_AUGMENTED_ENGINES,
	SPEC_AUTOMATED_REPAIR_UNIT,
	SPEC_BATTLE_PODS,
	SPEC_BATTLE_SCANNER,
	SPEC_CLOAKING_DEVICE,
	SPEC_DAMPER_FIELD,
	SPEC_DISPLACEMENT_DEVICE,
	SPEC_ECM_JAMMER,
	SPEC_ENERGY_ABSORBER,
	SPEC_EXTENDED_FUEL_TANKS,
	SPEC_FAST_MISSILE_RACKS,
	SPEC_HARD_SHIELDS,
	SPEC_HEAVY_ARMOR,
	SPEC_HIGH_ENERGY_FOCUS,
	SPEC_HYPERX_CAPACITORS,
	SPEC_INERTIAL_NULLIFIER,
	SPEC_INERTIAL_STABILIZER,
	SPEC_LIGHTNING_FIELD,
	SPEC_MULTIPHASED_SHIELDS,
	SPEC_MULTIWAVE_ECM_JAMMER,
	SPEC_PHASE_SHIFTER,
	SPEC_PHASING_CLOAK,
	SPEC_QUANTUM_DETONATOR,
	SPEC_RANGEMASTER_UNIT,
	SPEC_REFLECTION_FIELD,
	SPEC_REINFORCED_HULL,
	SPEC_SCOUT_LAB,
	SPEC_SECURITY_STATIONS,
	SPEC_SHIELD_CAPACITOR,
	SPEC_STEALTH_FIELD,
	SPEC_STRUCTURAL_ANALYZER,
	SPEC_SUB_SPACE_TELEPORTER,
	SPEC_TIME_WARP_FACILITATOR,
	SPEC_TRANSPORTERS,
	SPEC_TROOP_PODS,
	SPEC_WARP_DISSIPATOR,
	SPEC_WIDE_AREA_JAMMER,
	SPEC_REGENERATION
};

enum SelectionFilter {
	SELFILTER_NONE = 0,
	SELFILTER_OWNED,
	SELFILTER_ANY
};

extern const unsigned galaxySizeFactors[GALAXY_ZOOM_LEVELS];

class Fleet;
class GameState;

typedef int (*gamestate_cmp_func)(const GameState *gamestate, int player,
	unsigned a, unsigned b);

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

	void validate(void) const;
};

struct Galaxy {
	uint8_t sizeFactor;
	uint16_t width;
	uint16_t height;
	Nebula nebulas[MAX_NEBULAS];
	uint8_t nebulaCount;

	Galaxy(void);

	void load(ReadStream &stream);

	void validate(void) const;
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

	void validate(void) const;
};

struct Planet {
	int16_t colony;
	uint8_t star;
	uint8_t orbit;
	uint8_t type;
	uint8_t size;
	uint8_t gravity;
	uint8_t unknown1;

	// The effective climate of colonized planets may be different
	// from this value. Use GameState::planetClimate() to get the right
	// value.
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

	unsigned baseProduction(void) const;

	void validate(void) const;
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
	int8_t playerIndex;

	Leader(void);

	void load(ReadStream &stream);

	unsigned expLevel(void) const;
	const char *rank(void) const;

	// returns 1 if the leader has normal skill
	// returns 2 if the leader has advanced skill (usually +50% value)
	int hasSkill(unsigned id) const;

	// 0 if the leader doesn't have it
	int skillBonus(unsigned id) const;

	void validate(void) const;

	// Number of skills under given type
	static unsigned skillCount(unsigned type);

	// Find skill name for given ID
	static const char *skillName(unsigned id, unsigned advanced = 0);

	// Find skill number (e.g. icon ID) for given skill ID
	static unsigned skillNum(unsigned id);
};

struct ShipWeapon {
	int16_t type;
	uint8_t maxCount, workingCount;
	uint8_t arc;
	uint16_t mods;
	uint8_t ammo;

	void load(ReadStream &stream);

	unsigned arcID(void) const;
	const char *arcAbbr(void) const;
};

struct ShipDesign {
	char name[SHIP_NAME_SIZE];
	uint8_t size, type;
	uint8_t shield, drive, speed, computer, armor;
	uint8_t specials[(MAX_SHIP_SPECIALS + 7) / 8];

	// Colony and outpost ships use weapons[0].type to store destination
	// planet ID if sent directly to specific planet
	ShipWeapon weapons[MAX_SHIP_WEAPONS];
	uint8_t picture, builder;
	uint16_t cost;
	uint8_t baseCombatSpeed;
	uint16_t buildDate;

	ShipDesign(void);

	void load(ReadStream &stream);

	int hasSpecial(unsigned id) const;
	int hasWorkingSpecial(unsigned id, const uint8_t* specDamage) const;

	unsigned maxComputerHP(void) const;
	unsigned computerHP(unsigned compDamage) const;
	unsigned maxDriveHP(const uint8_t *specDamage = NULL) const;
	unsigned driveHP(unsigned driveDamage,
		const uint8_t *specDamage = NULL) const;

	unsigned combatSpeed(int transDimensional, unsigned driveDamage = 0,
		const uint8_t *specDamage = NULL) const;
	int beamOffense(unsigned compDamage = 0,
		const uint8_t *specDamage = NULL) const;
	// Cloaking device not included
	int beamDefense(int transDimensional, unsigned driveDamage = 0,
		const uint8_t *specDamage = NULL) const;

	void validate(void) const;
};

struct RaceTraits {
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
	uint16_t buildingMaintenance;
	uint16_t freighterMaintenance;
	uint16_t shipMaintenance;
	uint16_t spyMaintenance;
	uint16_t tributeCost;
	uint16_t officerMaintenance;

	// ResearchStatus enums
	uint8_t researchTopics[MAX_RESEARCH_TOPICS];
	uint8_t techs[MAX_TECHNOLOGIES];

	uint32_t researchProgress;

	uint8_t hyperTechLevels[MAX_RESEARCH_AREAS];

	uint8_t researchTopic;
	uint16_t researchItem;
	ShipDesign blueprints[MAX_PLAYER_BLUEPRINTS], selectedBlueprint;
	uint8_t playerContacts[MAX_PLAYERS];
	int8_t playerRelations[MAX_PLAYERS];
	uint8_t foreignPolicies[MAX_PLAYERS];	// ForeignPolicy enum
	uint8_t tradeTreaties[MAX_PLAYERS];
	uint8_t researchTreaties[MAX_PLAYERS];
	RaceTraits traits;

	// circular history graph buffers
	uint8_t fleetHistory[MAX_HISTORY_LENGTH];
	uint8_t techHistory[MAX_HISTORY_LENGTH];
	uint8_t populationHistory[MAX_HISTORY_LENGTH];
	uint8_t buildingHistory[MAX_HISTORY_LENGTH];

	uint8_t spies[MAX_PLAYERS];
	uint8_t galaxyCharted;

	Player(void);

	void load(SeekableReadStream &stream);

	int gravityPenalty(unsigned gravity) const;

	// returns 0/1 for applied techs, or current level for hyper techs
	unsigned knowsTechnology(unsigned tech_id) const;

	// returns 1 if the topic can be researched now
	int canResearchTopic(unsigned topic_id) const;

	// returns 1 if the technology can be researched at some point
	int canResearchTech(unsigned tech_id) const;

	unsigned blueprintCombatSpeed(unsigned id) const;
	unsigned blueprintCombatSpeed(const ShipDesign *design) const;
	int blueprintBeamOffense(unsigned id) const;
	int blueprintBeamOffense(const ShipDesign *design) const;
	int blueprintBeamDefense(unsigned id) const;
	int blueprintBeamDefense(const ShipDesign *design) const;

	int isPlayerVisible(unsigned player_id) const;

	void validate(void) const;
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
	// ID of the leader assigned by each player to their planets
	// in this star system. -1 = no leader.
	int8_t officerIndex[MAX_PLAYERS];
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

	// Find the sequential number for planet, ignoring empty orbits
	unsigned planetSeq(unsigned orbit) const;

	void validate(void) const;
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
	int hasSpecial(unsigned id) const;
	int hasWorkingSpecial(unsigned id) const;
	int isSpecialDamaged(unsigned id) const;

	unsigned maxComputerHP(void) const;
	unsigned computerHP(void) const;
	unsigned maxDriveHP(void) const;
	unsigned driveHP(void) const;

	int combatSpeed(int transDimensional, int ignoreDamage) const;
	int beamOffense(int ignoreDamage) const;
	// Cloaking Device not included
	int beamDefense(int transDimensional, int ignoreDamage) const;

	void validate(void) const;
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
	struct Player _players[MAX_PLAYERS];
	uint16_t _shipCount;
	Ship _ships[MAX_SHIPS];

	GameState(void);
	~GameState(void);

	void load(SeekableReadStream &stream);
	void load(const char *filename);
	void validate(void) const;
	void dump(void) const;

	// update cached values which depend on active player
	void setActivePlayer(unsigned player_id);

	unsigned findStar(int x, int y) const;
	BilistNode<Fleet> *getMovingFleets(void);
	const BilistNode<Fleet> *getMovingFleets(void) const;

	StarKnowledge isStarExplored(unsigned star_id,
		unsigned player_id) const;
	StarKnowledge isStarExplored(const Star *s, unsigned player_id) const;

	unsigned planetClimate(unsigned planet_id) const;
	unsigned planetMaxPop(unsigned planet_id, unsigned player_id) const;

	unsigned shipCombatSpeed(unsigned ship_id, int ignoreDamage) const;
	unsigned shipCombatSpeed(const Ship *sptr, int ignoreDamage) const;
	int shipBeamOffense(unsigned ship_id, int ignoreDamage) const;
	int shipBeamOffense(const Ship *sptr, int ignoreDamage) const;
	int shipBeamDefense(unsigned ship_id, int ignoreDamage) const;
	int shipBeamDefense(const Ship *sptr, int ignoreDamage) const;

	void sort_ids(unsigned *id_list, unsigned length, int player,
		gamestate_cmp_func cmp);
};

class Fleet : public Recyclable {
private:
	GameState *_parent;
	unsigned *_ships;
	size_t _shipCount, _maxShips;
	size_t _shipTypeCounts[MAX_SHIP_TYPES];
	size_t _combatCounts[MAX_COMBAT_SHIP_CLASSES];
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
	size_t shipTypeCount(unsigned type) const;
	size_t combatClassCount(unsigned cls) const;
	uint8_t getOwner(void) const;

	// Returns player[owner].color for player fleets, owner ID for monsters
	uint8_t getColor(void) const;

	uint8_t getStatus(void) const;
	uint16_t getX(void) const;
	uint16_t getY(void) const;
};

int cmpPlanetClimate(const GameState *game, int player, unsigned a,
	unsigned b);
int cmpPlanetMinerals(const GameState *game, int player, unsigned a,
	unsigned b);
int cmpPlanetMaxPop(const GameState *game, int player, unsigned a, unsigned b);

#endif
