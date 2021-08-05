/*
 * This file is part of OpenOrion2
 * Copyright (C) 2021 Martin Doucha
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

#include <cstdio>
#include <stdexcept>
#include <SDL.h>
#include "screen.h"
#include "lbx.h"
#include "savegame.h"
#include "mainmenu.h"
#include "system.h"

AssetManager *gameAssets = NULL;
FontManager gameFonts;

unsigned buttonState(unsigned sdlButtons) {
	unsigned ret = 0;

	if (sdlButtons & SDL_BUTTON_LMASK) {
		ret |= 1 << MBUTTON_LEFT;
	}

	if (sdlButtons & SDL_BUTTON_RMASK) {
		ret |= 1 << MBUTTON_RIGHT;
	}

	if (sdlButtons & ~(SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
		ret |= 1 << MBUTTON_OTHER;
	}

	return ret;
}

unsigned convertButton(unsigned sdlButton) {
	switch (sdlButton) {
	case SDL_BUTTON_LEFT:
		return MBUTTON_LEFT;

	case SDL_BUTTON_RIGHT:
		return MBUTTON_RIGHT;

	default:
		return MBUTTON_OTHER;
	}
}

void main_loop(void) {
	SDL_Event ev;
	GuiView *view, *prev_view = NULL;

	while (!gui_stack->is_empty()) {
		view = gui_stack->top();

		if (view != prev_view) {
			if (prev_view) {
				prev_view->close();
			}

			view->open();
			prev_view = view;
		}

		GarbageCollector::flush();

		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				view->close();
				gui_stack->clear();
				return;

			case SDL_MOUSEMOTION:
				view->handleMouseMove(ev.motion.x, ev.motion.y,
					buttonState(ev.motion.state));
				break;

			case SDL_MOUSEBUTTONDOWN:
				view->handleMouseDown(ev.button.x, ev.button.y,
					convertButton(ev.button.button));
				break;

			case SDL_MOUSEBUTTONUP:
				view->handleMouseUp(ev.button.x, ev.button.y,
					convertButton(ev.button.button));
				break;
			}
		}

		view->redraw(SDL_GetTicks());
		updateScreen();
		SDL_Delay(10);
	}
}

void dump(const GameState* state) {
    fprintf(stdout, "=== Config ===\n");
    fprintf(stdout, "Version: %d\n", state->_gameConfig.version);
    fprintf(stdout, "Save game name: %s\n", state->_gameConfig.saveGameName);
    fprintf(stdout, "Stardate: %d\n", state->_gameConfig.stardate);
    fprintf(stdout, "End of turn summary: %d\n", state->_gameConfig.endOfTurnSummary);
    fprintf(stdout, "End of turn wait: %d\n", state->_gameConfig.endOfTurnWait);
    fprintf(stdout, "Random events: %d\n", state->_gameConfig.randomEvents);
    fprintf(stdout, "Enemy moves: %d\n", state->_gameConfig.enemyMoves);
    fprintf(stdout, "Expanding help: %d\n", state->_gameConfig.expandingHelp);
    fprintf(stdout, "Autoselect ships: %d\n", state->_gameConfig.autoSelectShips);
    fprintf(stdout, "Animations: %d\n", state->_gameConfig.animations);
    fprintf(stdout, "Auto select colony: %d\n", state->_gameConfig.autoSelectColony);
    fprintf(stdout, "Show relocation lines: %d\n", state->_gameConfig.showRelocationLines);
    fprintf(stdout, "Show GNN Report: %d\n", state->_gameConfig.showGNNReport);
    fprintf(stdout, "Auto delete trade good housing: %d\n", state->_gameConfig.autoDeleteTradeGoodHousing);
    fprintf(stdout, "Show only serious turn summary: %d\n", state->_gameConfig.showOnlySeriousTurnSummary);
    fprintf(stdout, "Ship initiative: %d\n", state->_gameConfig.shipInitiative);

    fprintf(stdout, "=== Galaxy ===\n");
    fprintf(stdout, "Size factor: %d\n", state->_galaxy.sizeFactor);
    fprintf(stdout, "width: %d\n", state->_galaxy.width);
    fprintf(stdout, "height: %d\n", state->_galaxy.height);

    fprintf(stdout, "\n=== Hero ===\n");
    for (int i = 0; i < LEADER_COUNT; i++) {
        fprintf(stdout, "Name: %s\n", state->_leaders[i].name);
        fprintf(stdout, "Title: %s\n", state->_leaders[i].title);
        fprintf(stdout, "Type: %d\n", state->_leaders[i].type);
        fprintf(stdout, "Experience: %d\n", state->_leaders[i].experience);
        fprintf(stdout, "Common skills: %lu\n", state->_leaders[i].commonSkills);
        fprintf(stdout, "Special skills: %lu\n", state->_leaders[i].specialSkills);
        for (int j = 0; j < MAX_LEADER_TECH_SKILLS; j++) {
            fprintf(stdout, "Tech: %d\n", state->_leaders[i].techs[j]);
        }
        fprintf(stdout, "Picture: %d\n", state->_leaders[i].picture);
        fprintf(stdout, "Skill value: %d\n", state->_leaders[i].skillValue);
        fprintf(stdout, "Level: %d\n", state->_leaders[i].level);
        fprintf(stdout, "Location: %d\n", state->_leaders[i].location);
        fprintf(stdout, "ETA: %d\n", state->_leaders[i].eta);
        fprintf(stdout, "Level up: %d\n", state->_leaders[i].displayLevelUp);
        fprintf(stdout, "Status: %d\n", state->_leaders[i].status);
        fprintf(stdout, "Player: %d\n", state->_leaders[i].playerIndex);
    }

    fprintf(stdout, "\n=== Player ===\n");
    for (int i = 0; i < PLAYER_COUNT; i++) {
        fprintf(stdout, "Name:\t%s\tRace:\t%s\n",
            state->_players[i].name, state->_players[i].race);
        fprintf(stdout, "Picture:\t\t%d\tColor:\t\t\t%d\tPersonality:\t\t%d\n",
            state->_players[i].picture, state->_players[i].color, state->_players[i].personality);
        fprintf(stdout, "Objective:\t\t%d\tTax rate:\t\t%d\tBC:\t\t\t%lu\n",
            state->_players[i].objective, state->_players[i].taxRate, state->_players[i].BC);
        fprintf(stdout, "Total freighters:\t%d\tUsed freighters:\t%d\tCommand points:\t\t%d\n",
            state->_players[i].totalFreighters, state->_players[i].surplusFreighters, state->_players[i].commandPoints);
        fprintf(stdout, "Total production:\t%d\tRP:\t\t\t%d\tFood:\t\t\t%d\n",
            state->_players[i].industryProduced, state->_players[i].researchProduced, state->_players[i].surplusFood);
        fprintf(stdout, "Yearly BC:\t\t%d\tResearch progress:\t%d\tResearch Area:\t\t%d\n",
            state->_players[i].surplusBC, state->_players[i].researchProduced, state->_players[i].researchArea);
        fprintf(stdout, "Research Item:\t\t%d\n",
            state->_players[i].researchItem);

        fprintf(stdout, "--- Racepicks ---\n");
        fprintf(stdout, "Government:\t\t%d\tPopulation:\t\t%d\tFarming:\t\t%d\tScience:\t\t%d\n",
            state->_players[i].racePicks.government, state->_players[i].racePicks.population,
            state->_players[i].racePicks.farming, state->_players[i].racePicks.science);
        fprintf(stdout, "Money:\t\t\t%d\tShip defense:\t\t%d\tShip attack:\t\t%d\tGround combat:\t\t%d\n",
            state->_players[i].racePicks.money, state->_players[i].racePicks.shipDefense,
            state->_players[i].racePicks.shipAttack, state->_players[i].racePicks.groundCombat);
        fprintf(stdout, "Spying:\t\t\t%d\tLow G:\t\t\t%d\tHigh G:\t\t\t%d\tAquatic:\t\t%d\n",
            state->_players[i].racePicks.spying, state->_players[i].racePicks.lowG,
            state->_players[i].racePicks.highG, state->_players[i].racePicks.aquatic);
        fprintf(stdout, "Subterranian:\t\t%d\tLarge homeworld:\t%d\tRich/Poor homeworld:\t%d\tArtifacts homeworld:\t%d\n",
            state->_players[i].racePicks.subterranean, state->_players[i].racePicks.largeHomeworld,
            state->_players[i].racePicks.richHomeworld, state->_players[i].racePicks.artifactsHomeworld);
        fprintf(stdout, "Cybernetic:\t\t%d\tLithovore:\t\t%d\tRepulsive:\t\t%d\tCharismatic:\t\t%d\n",
            state->_players[i].racePicks.cybernetic, state->_players[i].racePicks.lithovore,
            state->_players[i].racePicks.repulsive, state->_players[i].racePicks.charismatic);
        fprintf(stdout, "Uncreative:\t\t%d\tCreative:\t\t%d\tTolerant:\t\t%d\tFantastic traders:\t%d\n",
            state->_players[i].racePicks.uncreative, state->_players[i].racePicks.creative,
            state->_players[i].racePicks.tolerant, state->_players[i].racePicks.fantasticTraders);
        fprintf(stdout, "Telepathic:\t\t%d\tLucky:\t\t\t%d\tOmniscience:\t\t%d\tStealthy ships:\t\t%d\n",
            state->_players[i].racePicks.telepathic, state->_players[i].racePicks.lucky,
            state->_players[i].racePicks.omniscience, state->_players[i].racePicks.stealthyShips);
        fprintf(stdout, "Transdimensional:\t%d\tWarlord:\t\t%d\n",
            state->_players[i].racePicks.transDimensional, state->_players[i].racePicks.warlord);
    }

    fprintf(stdout, "Number of stars: %d\n", state->_starSystemCount);
    for (auto star : state->_starSystems) {
        fprintf(stdout, "\nName:\t%s\n", star.name);
        fprintf(stdout, "Class:\t\t%x\t\tSize:\t\t%x\n", star.spectralClass, star.size);
        fprintf(stdout, "Position:\t%d,%d\tPrimary owner:\t%d\n", star.x, star.y, star.owner);
        fprintf(stdout, "Special:\t%d\t\tWormhole:\t%d\n", star.special, star.wormhole);
    }
}

void prepare_main_menu(void) {
	ImageAsset bg, anim;
	GuiView *view = NULL;

	try {
		view = new MainMenuView;
		gui_stack->push(view);
		view = NULL;
		bg = gameAssets->getImage("mainmenu.lbx", 1);
		anim = gameAssets->getImage("mainmenu.lbx", 0, bg->palette());
		view = new TransitionView((Image*)bg, (Image*)anim);
		gui_stack->push(view);
		view = NULL;
		anim = gameAssets->getImage("logo.lbx", 1);
		view = new TransitionView(NULL, (Image*)anim);
		gui_stack->push(view);
	} catch (...) {
		delete view;
		throw;
	}
}

void engine_shutdown(void) {
	delete gui_stack;
	GarbageCollector::flush();
	gameFonts.clear();
	delete gameAssets;
	shutdownScreen();
	cleanup_datadir();
}

int main(int argc, char **argv) {
	if (argc >= 2) {
		GameState* game = Savegame::load(argv[1]);
		dump(game);
	}

	try {
		init_datadir(argv[0]);
		gameAssets = new AssetManager;
		gui_stack = new ViewStack;
		initScreen();
		// TODO: add multilingual support
		load_fonts("fonts.lbx");
	} catch(std::exception &e) {
		fprintf(stderr, "Error: %s\n", e.what());
		engine_shutdown();
		return 1;
	}

	try {
		prepare_main_menu();
		main_loop();
	} catch(std::exception &e) {
		fprintf(stderr, "Error: %s\n", e.what());
		engine_shutdown();
		return 1;
	}

	engine_shutdown();
	return 0;
}
