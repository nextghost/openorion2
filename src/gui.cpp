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

#include <cstring>
#include "gui.h"

GuiCallback::GuiCallback(void) : _callback(NULL) {

}

GuiCallback::GuiCallback(const GuiCallback &other) : _callback(other.copy()) {

}

GuiCallback::~GuiCallback(void) {
	delete _callback;
}

const GuiCallback &GuiCallback::operator=(const GuiCallback &other) {
	delete _callback;
	_callback = other.copy();
	return *this;
}

void GuiCallback::operator()(int x, int y) {
	if (_callback) {
		(*_callback)(x, y);
	}
}

GuiCallback *GuiCallback::copy(void) const {
	if (_callback) {
		return _callback->copy();
	}

	return NULL;
}

ActiveZone::ActiveZone(unsigned left, unsigned top, unsigned w, unsigned h) :
	x(left), y(top), width(w), height(h), state(ZSTATE_IDLE) {

}

GuiView::GuiView(void) : _zones(NULL), _currentZone(NULL), _zoneCount(0),
	_zoneMax(32) {
	_zones = new ActiveZone*[_zoneMax];
}

GuiView::~GuiView(void) {
	size_t i;

	for (i = 0; i < _zoneCount; i++) {
		delete _zones[i];
	}

	delete[] _zones;
}

void GuiView::addZone(ActiveZone *zone) {
	if (_zoneCount >= _zoneMax) {
		size_t size = 2 * _zoneMax;
		ActiveZone **tmp = new ActiveZone*[size];

		memcpy(tmp, _zones, _zoneCount * sizeof(ActiveZone*));
		delete[] _zones;
		_zones = tmp;
		_zoneMax = size;
	}

	_zones[_zoneCount++] = zone;
}

ActiveZone *GuiView::findZone(int x, int y) {
	size_t i;
	ActiveZone **ptr;

	if (x < 0 || y < 0) {
		return NULL;
	}

	for (i = 0, ptr = _zones; i < _zoneCount; i++, ptr++) {
		if (x >= (*ptr)->x && x < (*ptr)->x + (*ptr)->width &&
			y >= (*ptr)->y && y < (*ptr)->y + (*ptr)->height) {
			return *ptr;
		}
	}

	return NULL;
}

void GuiView::exitView(void) {
	close();
	// TODO: discard this view and switch to the next one (if any)
}

void GuiView::open(void) {

}

void GuiView::close(void) {

}

void GuiView::handleMouseMove(int x, int y, unsigned buttons) {
	ActiveZone *zone = findZone(x, y);

	if (_currentZone != zone) {
		if (_currentZone) {
			_currentZone->onMouseOut(x, y);
			_currentZone->state = ZSTATE_IDLE;
		}

		if (zone) {
			zone->state = ZSTATE_MOUSEOVER;

			if (buttons & MBUTTON_LEFT) {
				zone->state |= ZSTATE_LEFTCLICK;
			}

			if (buttons & MBUTTON_RIGHT) {
				zone->state |= ZSTATE_RIGHTCLICK;
			}

			zone->onMouseOver(x, y);
		}

		_currentZone = zone;
	} else if (zone) {
		zone->onMouseMove(x, y);
	}
}

void GuiView::handleMouseDown(int x, int y, unsigned button) {
	if (!_currentZone) {
		_currentZone = findZone(x, y);
	}

	if (!_currentZone) {
		return;
	}

	switch (button) {
	case MBUTTON_LEFT:
		_currentZone->onMouseDown(x, y);
		_currentZone->state |= ZSTATE_LEFTCLICK;
		break;

	case MBUTTON_RIGHT:
		_currentZone->onMouseRightDown(x, y);
		_currentZone->state |= ZSTATE_RIGHTCLICK;
		break;
	}
}

void GuiView::handleMouseUp(int x, int y, unsigned button) {
	if (!_currentZone) {
		_currentZone = findZone(x, y);
	}

	if (!_currentZone) {
		return;
	}

	switch (button) {
	case MBUTTON_LEFT:
		_currentZone->onMouseUp(x, y);
		_currentZone->state &= ~ZSTATE_LEFTCLICK;
		break;

	case MBUTTON_RIGHT:
		_currentZone->onMouseRightUp(x, y);
		_currentZone->state &= ~ZSTATE_RIGHTCLICK;
		break;
	}
}

TransitionView::TransitionView(Image *background, Image *animation, int x,
	int y) : _background(background), _animation(animation), _x(x), _y(y),
	_startTick(0) {

}

TransitionView::~TransitionView(void) {

}

void TransitionView::redraw(unsigned curtick) {
	unsigned frame, frameTime;

	if (!_startTick) {
		_startTick = curtick;
	}

	if (_background) {
		_background->draw(0, 0);
	}

	if (_animation) {
		frameTime = _animation->frameTime();
		frameTime = frameTime < 10 ? 10 : frameTime;
		frame = (curtick - _startTick) / frameTime;

		if (frame >= _animation->frameCount()) {
			frame = _animation->frameCount() - 1;
			exitView();
		}

		_animation->draw(_x, _y, frame);
	}
}

void TransitionView::handleMouseMove(int x, int y, unsigned buttons) {

}

void TransitionView::handleMouseDown(int x, int y, unsigned button) {

}

void TransitionView::handleMouseUp(int x, int y, unsigned button) {
	exitView();
}
