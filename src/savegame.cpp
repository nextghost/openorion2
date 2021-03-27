#include <stdexcept>
#include "savegame.h"

const int LEADERS_DATA_OFFSET 		= 0x019a9b;
const int LEADER_TYPE_OFFSET		= 0x24;

const int STARS_COUNT_DATA_OFFSET 	= 0x017ad1;
const int STARS_DATA_OFFSET 		= 0x017ad3;

const int PLAYERS_DATA_OFFSET 		= 0x01aa0f;

GameState* Savegame::load(const char *fileName) {
    File file;
    GameState* gameState;

    if (!file.open(fileName)) {
        throw std::runtime_error("Cannot open savegame file");
    }

    MemoryReadStream* stream = file.readStream(file.size());
    file.close();

    gameState = new GameState;
    readConfig(*stream, gameState);
    readGalaxy(*stream, gameState);
    readStars(*stream, gameState);
    readLeaders(*stream, gameState);
    readPlayers(*stream, gameState);
    delete stream;

    return gameState;
}

void Savegame::readConfig(SeekableReadStream& stream, GameState* state) {
    state->_gameConfig.version = stream.readUint32LE();
    stream.read(state->_gameConfig.saveGameName, SAVE_GAME_NAME_SIZE);
    state->_gameConfig.stardate = stream.readUint16LE();

    stream.seek(0x2E, SEEK_SET);
    state->_gameConfig.endOfTurnSummary = stream.readUint8();
    state->_gameConfig.endOfTurnWait = stream.readUint8();
    state->_gameConfig.randomEvents = stream.readUint8();
    state->_gameConfig.enemyMoves = stream.readUint8();
    state->_gameConfig.expandingHelp = stream.readUint8();
    state->_gameConfig.autoSelectShips = stream.readUint8();
    state->_gameConfig.animations = stream.readUint8();
    state->_gameConfig.autoSelectColony = stream.readUint8();
    state->_gameConfig.showRelocationLines = stream.readUint8();
    state->_gameConfig.showGNNReport = stream.readUint8();
    state->_gameConfig.autoDeleteTradeGoodHousing = stream.readUint8();
    state->_gameConfig.showOnlySeriousTurnSummary = stream.readUint8();
    state->_gameConfig.shipInitiative = stream.readUint8();
}

void Savegame::readGalaxy(SeekableReadStream& stream, GameState* state) {
    stream.seek(0x31be4, SEEK_SET);
    state->_galaxy.sizeFactor = stream.readUint8();
    stream.readUint32LE(); // Skip unknown data
    state->_galaxy.width = stream.readUint16LE();
    state->_galaxy.height = stream.readUint16LE();
}

void Savegame::readLeaders(SeekableReadStream& stream, GameState* state) {
    int offset = 0;
    for (int i = 0; i < LEADER_COUNT; i++) {
        offset = LEADERS_DATA_OFFSET + (LEADER_RECORD_SIZE * i);

        stream.seek(offset, SEEK_SET);
        stream.read(state->_leaders[i].name, LEADER_NAME_SIZE);

        stream.seek(offset+LEADER_NAME_SIZE, SEEK_SET);
        stream.read(state->_leaders[i].title, LEADER_TITLE_SIZE);

        stream.seek(offset+LEADER_TYPE_OFFSET, SEEK_SET);
        state->_leaders[i].type = stream.readUint8();
        state->_leaders[i].experience = stream.readUint16LE();

        state->_leaders[i].commonSkills = stream.readUint64LE();
        state->_leaders[i].specialSkills = stream.readUint64LE();
        for (int j = 0; j < MAX_LEADER_TECH_SKILLS; j++) {
            state->_leaders[i].techs[j] = stream.readUint8();
        }
        state->_leaders[i].picture = stream.readUint8();
        state->_leaders[i].skillValue = stream.readUint16LE();
        state->_leaders[i].level = stream.readUint8();
        state->_leaders[i].location = stream.readUint16LE();
        state->_leaders[i].eta = stream.readUint8();
        state->_leaders[i].displayLevelUp = stream.readUint8();
        state->_leaders[i].status = stream.readUint8();
        state->_leaders[i].playerIndex = stream.readUint8();
    }
}

void Savegame::readPlayers(SeekableReadStream& stream, GameState* state) {
    const int PLAYERS_RECORD_SIZE = 3753;
    int offset = 0;

    for (int i = 0; i < PLAYER_COUNT; i++) {
        offset = PLAYERS_DATA_OFFSET + (PLAYERS_RECORD_SIZE * i);

        stream.seek(offset, SEEK_SET);
        stream.read(state->_players[i].name, PLAYER_NAME_SIZE);
        stream.read(state->_players[i].race, PLAYER_RACE_SIZE);
        state->_players[i].picture = stream.readUint8();
        state->_players[i].color = stream.readUint8();
        state->_players[i].personality = stream.readUint8();
        // 100 = Human player
        state->_players[i].objective = stream.readUint8();
        state->_players[i].homePlayerId = stream.readUint16LE();
        state->_players[i].networkPlayerId = stream.readUint16LE();
        state->_players[i].playerDoneFlags = stream.readUint8();
        stream.readUint16LE(); // Dead field
        state->_players[i].researchBreakthrough = stream.readUint8();
        state->_players[i].taxRate = stream.readUint8();
        state->_players[i].BC = stream.readUint32LE();
        state->_players[i].totalFreighters = stream.readUint16LE();
        state->_players[i].surplusFreighters = stream.readUint16LE();
        state->_players[i].commandPoints = stream.readUint16LE();
        state->_players[i].usedCommandPoints = stream.readSint16LE();
        state->_players[i].foodFreighted = stream.readUint16LE();
        state->_players[i].settlersFreighted = stream.readUint16LE();

        BitStream bitstream(stream);
        for (int j = 0; j < MAX_SETTLERS; j++) {
            state->_players[i].settlers[j].sourceColony = bitstream.readBitsLE(8);
            state->_players[i].settlers[j].destinationPlanet = bitstream.readBitsLE(8);
            state->_players[i].settlers[j].player = bitstream.readBitsLE(4);
            state->_players[i].settlers[j].eta = bitstream.readBitsLE(4);
            state->_players[i].settlers[j].job = bitstream.readBitsLE(2);
            bitstream.readBitsLE(6);
        }

        state->_players[i].totalPop = stream.readUint16LE();
        state->_players[i].foodProduced = stream.readUint16LE();
        state->_players[i].industryProduced = stream.readUint16LE();
        state->_players[i].researchProduced = stream.readUint16LE();
        state->_players[i].bcProduced = stream.readUint16LE();

        state->_players[i].surplusFood = stream.readSint16LE();
        state->_players[i].surplusBC = stream.readSint16LE();

        state->_players[i].totalMaintenance = stream.readSint32LE();
        state->_players[i].researchArea = ResearchArea(stream.readUint8());
        state->_players[i].researchItem = stream.readUint8();

        stream.seek(offset + 0x89E, SEEK_SET);
        state->_players[i].racePicks.government = stream.readUint8();
        state->_players[i].racePicks.population = stream.readSint8();
        state->_players[i].racePicks.farming = stream.readSint8();
        state->_players[i].racePicks.industry = stream.readSint8();
        state->_players[i].racePicks.science = stream.readSint8();
        state->_players[i].racePicks.money = stream.readSint8();
        state->_players[i].racePicks.shipDefense = stream.readSint8();
        state->_players[i].racePicks.shipAttack = stream.readSint8();
        state->_players[i].racePicks.groundCombat = stream.readSint8();
        state->_players[i].racePicks.spying = stream.readSint8();
        state->_players[i].racePicks.lowG = stream.readUint8();
        state->_players[i].racePicks.highG = stream.readUint8();
        state->_players[i].racePicks.aquatic = stream.readUint8();
        state->_players[i].racePicks.subterranean = stream.readUint8();
        state->_players[i].racePicks.largeHomeworld = stream.readUint8();
        state->_players[i].racePicks.richHomeworld = stream.readSint8();
        stream.seek(offset + 0x8ae, SEEK_SET);
        state->_players[i].racePicks.artifactsHomeworld = stream.readUint8();
        state->_players[i].racePicks.cybernetic = stream.readUint8();
        state->_players[i].racePicks.lithovore = stream.readUint8();
        state->_players[i].racePicks.repulsive = stream.readUint8();
        state->_players[i].racePicks.charismatic = stream.readUint8();
        state->_players[i].racePicks.uncreative = stream.readUint8();
        state->_players[i].racePicks.creative = stream.readUint8();
        state->_players[i].racePicks.tolerant = stream.readUint8();
        state->_players[i].racePicks.fantasticTraders = stream.readUint8();
        state->_players[i].racePicks.telepathic = stream.readUint8();
        state->_players[i].racePicks.lucky = stream.readUint8();
        state->_players[i].racePicks.omniscience = stream.readUint8();
        state->_players[i].racePicks.stealthyShips = stream.readUint8();
        state->_players[i].racePicks.transDimensional = stream.readUint8();
        state->_players[i].racePicks.warlord = stream.readUint8();
    }
}

void Savegame::readStars(SeekableReadStream& stream, GameState* state) {
    int offset = STARS_COUNT_DATA_OFFSET;
    stream.seek(offset, SEEK_SET);
    state->_starSystemCount = stream.readUint8();

    for (int i = 0; i < state->_starSystemCount; i++) {
        int record_offset = STARS_DATA_OFFSET + STARS_RECORD_SIZE * i;
        stream.seek(record_offset, SEEK_SET);
        stream.read(state->_starSystems[i].name, STARS_NAME_SIZE);
        state->_starSystems[i].x = stream.readUint16LE();
        state->_starSystems[i].y = stream.readUint16LE();
        state->_starSystems[i].size = StarSize(stream.readUint8());
        state->_starSystems[i].owner = stream.readUint8();
        state->_starSystems[i].pictureType = stream.readUint8();
        state->_starSystems[i].spectralClass = SpectralClass(stream.readUint8());

        for (int j = 0; j < (MAX_STARS + 7)/8; j++) {
            state->_starSystems[i].blackHoleBlocks[j] = stream.readUint8();
        }

        state->_starSystems[i].special = SpecialType(stream.readUint8());
        state->_starSystems[i].wormhole = stream.readUint8();
        state->_starSystems[i].blockaded = stream.readUint8();

        for (int j = 0; j < MAX_PLAYERS; j++) {
            state->_starSystems[i].blockadedBy[j] = stream.readUint8();
        }
        state->_starSystems[i].visited = stream.readUint8();
        state->_starSystems[i].justVisited = stream.readUint8();
        state->_starSystems[i].ignoreColonyShips = stream.readUint8();
        state->_starSystems[i].ignoreCombatShips = stream.readUint8();
        state->_starSystems[i].colonizePlayer = stream.readSint8();
        state->_starSystems[i].hasColony = stream.readUint8();
        state->_starSystems[i].hasWarpFieldInterdictor = stream.readUint8();
        state->_starSystems[i].nextWFIInList = stream.readUint8();
        state->_starSystems[i].hasTachyon = stream.readUint8();
        state->_starSystems[i].hasSubspace = stream.readUint8();
        state->_starSystems[i].hasStargate = stream.readUint8();
        state->_starSystems[i].hasJumpgate = stream.readUint8();
        state->_starSystems[i].hasArtemisNet = stream.readUint8();
        state->_starSystems[i].hasDimensionalPortal = stream.readUint8();
        state->_starSystems[i].isStagepoint = stream.readUint8();

        for (int j = 0; j < MAX_PLAYERS; j++) {
            state->_starSystems[i].officerIndex[j] = stream.readUint8();
        }
        for (int j = 0; j < MAX_PLANETS_PER_SYSTEM; j++) {
            state->_starSystems[i].planetIndex[j] = stream.readUint16LE();
        }
        for (int j = 0; j < MAX_PLAYERS; j++) {
            state->_starSystems[i].relocateShipTo[j] = stream.readUint16LE();
        }
        stream.readUint8();
        stream.readUint8();
        stream.readUint8();

        for (int j = 0; j < MAX_PLAYERS; j++) {
            state->_starSystems[i].surrenderTo[j] = stream.readUint8();
        }
        state->_starSystems[i].inNebula = stream.readUint8();
        state->_starSystems[i].artifactsGaveApp = stream.readUint8();
    }
}

