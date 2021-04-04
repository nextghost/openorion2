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
#include "screen.h"
#include "gui.h"

#define WSTATE_IDLE 0
#define WSTATE_MOUSEOVER 1
#define WSTATE_LEFTCLICK 2
#define WSTATE_RIGHTCLICK 4

#define WSPRITE_IDLE 0
#define WSPRITE_MOUSEOVER 1
#define WSPRITE_CLICK 2

#define MIN_FRAMETIME 10
#define DEFAULT_FRAMETIME 15

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

GuiSprite::GuiSprite(Image *img, int offsx, int offsy, int frame,
	unsigned imgx, unsigned imgy, unsigned width, unsigned height) :
	_image(img), _x(imgx), _y(imgy), _width(0), _height(0), _startTick(0),
	_offsx(offsx), _offsy(offsy), _frame(frame) {

	if (!_image) {
		throw std::runtime_error("Image is NULL");
	}

	if (_x >= _image->width() || _y >= _image->height()) {
		throw std::out_of_range("Subimage offset out of range");
	}

	if (_frame >= 0 && _frame > _image->frameCount()) {
		throw std::out_of_range("Image frame out of range");
	}

	gameAssets->takeImage(_image);
	_width = width ? width : _image->width() - _x;
	_height = height ? height : _image->height() - _y;
}

GuiSprite::~GuiSprite(void) {
	gameAssets->freeImage(_image);
}

void GuiSprite::startAnimation(void) {
	_startTick = 0;
}

void GuiSprite::stopAnimation(void) {

}

void GuiSprite::redraw(unsigned x, unsigned y, unsigned curtick) {
	unsigned fid, ftime, fcount;

	if (!_startTick) {
		_startTick = curtick;
	}

	if (_frame >= 0) {
		fid = _frame;
	} else {
		ftime = _image->frameTime();
		ftime = ftime < MIN_FRAMETIME ? DEFAULT_FRAMETIME : ftime;
		fid = ((curtick - _startTick) / ftime);
		fcount = _image->frameCount();

		if (fid >= fcount) {
			if (_frame == ANIM_ONCE) {
				return;
			}

			fid = _frame == ANIM_LOOP ? fid % fcount : fcount - 1;
		}
	}

	drawTextureTile(_image->textureID(fid), x + _offsx, y + _offsy, _x,
		_y, _width, _height);
}

Widget::Widget(unsigned x, unsigned y, unsigned width, unsigned height) :
	_x(x), _y(y), _width(width), _height(height), _state(WSTATE_IDLE),
	_cursprite(NULL) {

	memset(_sprites, 0, WIDGET_SPRITES * sizeof(GuiSprite*));
}

Widget::~Widget(void) {
	unsigned i;

	for (i = 0; i < WIDGET_SPRITES; i++) {
		delete _sprites[i];
	}
}

void Widget::changeSprite(void) {
	GuiSprite *newsprite = _sprites[WSPRITE_IDLE];
	unsigned i;

	for (i = 0; i < WIDGET_SPRITES - 1; i++) {
		if (_state & (1 << i) && _sprites[i + 1]) {
			newsprite = _sprites[i + 1];
		}
	}

	if (_cursprite && _cursprite != newsprite) {
		_cursprite->stopAnimation();
	}

	_cursprite = newsprite;

	if (_cursprite) {
		_cursprite->startAnimation();
	}
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

void Widget::setIdleSprite(GuiSprite *sprite) {
	if (_sprites[WSPRITE_IDLE]) {
		delete _sprites[WSPRITE_IDLE];
	}

	_sprites[WSPRITE_IDLE] = sprite;
}

void Widget::setIdleSprite(Image *img, int frame) {
	GuiSprite *sprite = new GuiSprite(img, 0, 0, frame);

	try {
		setIdleSprite(sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void Widget::setIdleSprite(const char *archive, unsigned id,
	const uint8_t *palette, int frame) {
	Image *img = gameAssets->getImage(archive, id, palette);

	try {
		setIdleSprite(img, frame);
	} catch (...) {
		gameAssets->freeImage(img);
		throw;
	}

	gameAssets->freeImage(img);
}

void Widget::setMouseOverSprite(GuiSprite *sprite) {
	if (_sprites[WSPRITE_MOUSEOVER]) {
		delete _sprites[WSPRITE_MOUSEOVER];
	}

	_sprites[WSPRITE_MOUSEOVER] = sprite;
}

void Widget::setMouseOverSprite(Image *img, int frame) {
	GuiSprite *sprite = new GuiSprite(img, 0, 0, frame);

	try {
		setMouseOverSprite(sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void Widget::setMouseOverSprite(const char *archive, unsigned id,
	const uint8_t *palette, int frame) {
	Image *img = gameAssets->getImage(archive, id, palette);

	try {
		setMouseOverSprite(img, frame);
	} catch (...) {
		gameAssets->freeImage(img);
		throw;
	}

	gameAssets->freeImage(img);
}

void Widget::setClickSprite(unsigned button, GuiSprite *sprite) {
	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	if (_sprites[WSPRITE_CLICK + button]) {
		delete _sprites[WSPRITE_CLICK + button];
	}

	_sprites[WSPRITE_CLICK + button] = sprite;
}

void Widget::setClickSprite(unsigned button, Image *img, int frame) {
	GuiSprite *sprite = new GuiSprite(img, 0, 0, frame);

	try {
		setClickSprite(button, sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void Widget::setClickSprite(unsigned button, const char *archive, unsigned id,
	const uint8_t *palette, int frame) {
	Image *img = gameAssets->getImage(archive, id, palette);

	try {
		setClickSprite(button, img, frame);
	} catch (...) {
		gameAssets->freeImage(img);
		throw;
	}

	gameAssets->freeImage(img);
}

void Widget::handleMouseOver(int x, int y, unsigned buttons) {
	_state = WSTATE_MOUSEOVER;

	if (buttons & (1 << MBUTTON_LEFT)) {
		_state |= WSTATE_LEFTCLICK;
	}

	if (buttons & (1 << MBUTTON_RIGHT)) {
		_state |= WSTATE_RIGHTCLICK;
	}

	_onMouseOver(x, y);
	changeSprite();
}

void Widget::handleMouseMove(int x, int y, unsigned buttons) {
	_onMouseMove(x, y);
}

void Widget::handleMouseOut(int x, int y, unsigned buttons) {
	_state = WSTATE_IDLE;
	_onMouseOut(x, y);
	changeSprite();
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
	changeSprite();
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
	changeSprite();
}

void Widget::redraw(unsigned curtick) {
	if (_cursprite) {
		_cursprite->redraw(_x, _y, curtick);
	}
}

GuiView::GuiView(void) : _widgets(NULL), _currentWidget(NULL), _widgetCount(0),
	_widgetMax(32) {
	_widgets = new Widget*[_widgetMax];
}

GuiView::~GuiView(void) {
	clearWidgets();
}

void GuiView::addWidget(Widget *w) {
	if (_widgetCount >= _widgetMax) {
		size_t size = _widgetMax ? 2 * _widgetMax : 32;
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

void GuiView::redrawWidgets(unsigned curtick) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		_widgets[i]->redraw(curtick);
	}
}

void GuiView::clearWidgets(void) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		delete _widgets[i];
	}

	delete[] _widgets;
	_widgets = NULL;
	_widgetCount = _widgetMax = 0;
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
		frameTime = frameTime < MIN_FRAMETIME ? DEFAULT_FRAMETIME :
			frameTime;
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
