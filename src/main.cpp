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
#include <clocale>
#include <SDL.h>
#include "screen.h"
#include "gamestate.h"
#include "mainmenu.h"
#include "galaxy.h"
#include "system.h"

AssetManager *gameAssets = NULL;
TextManager *gameLang = NULL;
FontManager *gameFonts = NULL;
Screen *gameScreen = NULL;

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
	delete gameFonts;
	delete gameLang;
	delete gameAssets;
	delete gameScreen;
	cleanup_paths();
}

int main(int argc, char **argv) {
	// Honor system locale
	setlocale(LC_ALL, "");

	try {
		init_paths(argv[0]);
		gameAssets = new AssetManager;
		gui_stack = new ViewStack;
		gameScreen = Screen::createScreen();
		// FIXME: Select language from game config
		selectLanguage(LANG_ENGLISH);
	} catch(std::exception &e) {
		fprintf(stderr, "Error: %s\n", e.what());
		engine_shutdown();
		return 1;
	}

	try {
		if (argc >= 2) {
			GameState* game = NULL;
			GuiView *view = NULL;

			try {
				game = new GameState;
				game->load(argv[1]);
				game->dump();
				view = new GalaxyView(game);
				game = NULL;
				gui_stack->push(view);
			} catch (std::exception &e) {
				delete game;
				delete view;
				throw;
			}

		} else {
			prepare_main_menu();
		}

		main_loop();
	} catch(std::exception &e) {
		fprintf(stderr, "Error: %s\n", e.what());
		engine_shutdown();
		return 1;
	}

	engine_shutdown();
	return 0;
}
