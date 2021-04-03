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
#include <stdexcept>
#include "lbx.h"
#include "gui.h"

#define WSTATE_IDLE 0
#define WSTATE_MOUSEOVER 1
#define WSTATE_LEFTCLICK 2
#define WSTATE_RIGHTCLICK 4

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

Widget::Widget(unsigned x, unsigned y, unsigned width, unsigned height) :
	_x(x), _y(y), _width(width), _height(height), _state(WSTATE_IDLE) {

}

Widget::~Widget(void) {

}

int Widget::isInside(unsigned x, unsigned y) const {
	return x >= _x && x < _x + _width && y >= _y && y < _y + _height;
}

void Widget::setMouseOverCallback(const GuiCallback &callback) {
	_onMouseOver = callback;
}

void Widget::setMouseMoveCallback(const GuiCallback &callback) {
	_onMouseMove = callback;
}

void Widget::setMouseOutCallback(const GuiCallback &callback) {
	_onMouseOut = callback;
}

void Widget::setMouseDownCallback(unsigned button,
	const GuiCallback &callback) {

	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	_onMouseDown[button] = callback;
}

void Widget::setMouseUpCallback(unsigned button, const GuiCallback &callback) {
	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	_onMouseUp[button] = callback;
}

void Widget::handleMouseOver(int x, int y, unsigned buttons) {
	_state = WSTATE_MOUSEOVER;

	if (buttons & MBUTTON_LEFT) {
		_state |= WSTATE_LEFTCLICK;
	}

	if (buttons & MBUTTON_RIGHT) {
		_state |= WSTATE_RIGHTCLICK;
	}

	_onMouseOver(x, y);
}

void Widget::handleMouseMove(int x, int y, unsigned buttons) {
	_onMouseMove(x, y);
}

void Widget::handleMouseOut(int x, int y, unsigned buttons) {
	_state = WSTATE_IDLE;
	_onMouseOut(x, y);
}

void Widget::handleMouseDown(int x, int y, unsigned button) {
	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	switch (button) {
	case MBUTTON_LEFT:
		_state |= WSTATE_LEFTCLICK;
		break;

	case MBUTTON_RIGHT:
		_state |= WSTATE_RIGHTCLICK;
		break;
	}

	_onMouseDown[button](x, y);
}

void Widget::handleMouseUp(int x, int y, unsigned button) {
	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	switch (button) {
	case MBUTTON_LEFT:
		_state &= ~WSTATE_LEFTCLICK;
		break;

	case MBUTTON_RIGHT:
		_state &= ~WSTATE_RIGHTCLICK;
		break;
	}

	_onMouseUp[button](x, y);
}

GuiView::GuiView(void) : _widgets(NULL), _currentWidget(NULL), _widgetCount(0),
	_widgetMax(32) {
	_widgets = new Widget*[_widgetMax];
}

GuiView::~GuiView(void) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		delete _widgets[i];
	}

	delete[] _widgets;
}

void GuiView::addWidget(Widget *w) {
	if (_widgetCount >= _widgetMax) {
		size_t size = 2 * _widgetMax;
		Widget **tmp = new Widget*[size];

		memcpy(tmp, _widgets, _widgetCount * sizeof(Widget*));
		delete[] _widgets;
		_widgets = tmp;
		_widgetMax = size;
	}

	_widgets[_widgetCount++] = w;
}

Widget *GuiView::findWidget(int x, int y) {
	size_t i;
	Widget **ptr;

	if (x < 0 || y < 0) {
		return NULL;
	}

	for (i = 0, ptr = _widgets; i < _widgetCount; i++, ptr++) {
		if ((*ptr)->isInside(x, y)) {
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
	Widget *w = findWidget(x, y);

	if (_currentWidget != w) {
		if (_currentWidget) {
			_currentWidget->handleMouseOut(x, y, buttons);
		}

		if (w) {
			w->handleMouseOver(x, y, buttons);
		}

		_currentWidget = w;
	}

	if (w) {
		w->handleMouseMove(x, y, buttons);
	}
}

void GuiView::handleMouseDown(int x, int y, unsigned button) {
	if (!_currentWidget) {
		_currentWidget = findWidget(x, y);
	}

	if (_currentWidget) {
		_currentWidget->handleMouseDown(x, y, button);
	}
}

void GuiView::handleMouseUp(int x, int y, unsigned button) {
	if (!_currentWidget) {
		_currentWidget = findWidget(x, y);
	}

	if (_currentWidget) {
		_currentWidget->handleMouseUp(x, y, button);
	}
}

TransitionView::TransitionView(Image *background, Image *animation, int x,
	int y) : _background(background), _animation(animation), _x(x), _y(y),
	_startTick(0) {

	gameAssets->takeImage(_background);
	gameAssets->takeImage(_animation);
}

TransitionView::~TransitionView(void) {
	gameAssets->freeImage(_background);
	gameAssets->freeImage(_animation);
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
