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
	cleanup_paths();
}

int main(int argc, char **argv) {
	if (argc >= 2) {
		GameState* game = NULL;

		try {
			game = new GameState;
			game->load(argv[1]);
			game->dump();
		} catch (std::exception &e) {
			fprintf(stderr, "Error reading saved game: %s\n",
				e.what());
		}

		delete game;
	}

	try {
		init_paths(argv[0]);
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
