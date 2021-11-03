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

ViewStack *gui_stack = NULL;

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

	if (_frame >= 0 && unsigned(_frame) > _image->frameCount()) {
		throw std::out_of_range("Image frame out of range");
	}

	gameAssets->takeAsset(_image);
	_width = width ? width : _image->width() - _x;
	_height = height ? height : _image->height() - _y;
}

GuiSprite::GuiSprite(const GuiSprite &other) : _image(other._image),
	_x(other._x), _y(other._y), _width(other._width), _startTick(0),
	_offsx(other._offsx), _offsy(other._offsy), _frame(other._frame) {

	gameAssets->takeAsset(_image);
}

GuiSprite::~GuiSprite(void) {
	gameAssets->freeAsset(_image);
}

const GuiSprite &GuiSprite::operator=(const GuiSprite &other) {
	gameAssets->takeAsset(other._image);

	try {
		gameAssets->freeAsset(_image);
	} catch (...) {
		gameAssets->freeAsset(other._image);
		throw;
	}

	_image = other._image;
	_x = other._x;
	_y = other._y;
	_width = other._width;
	_startTick = 0;
	_offsx = other._offsx;
	_offsy = other._offsy;
	_frame = other._frame;
	return *this;
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
	changeSprite();
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
	ImageAsset img = gameAssets->getImage(archive, id, palette);

	setIdleSprite((Image*)img, frame);
}

void Widget::setMouseOverSprite(GuiSprite *sprite) {
	if (_sprites[WSPRITE_MOUSEOVER]) {
		delete _sprites[WSPRITE_MOUSEOVER];
	}

	_sprites[WSPRITE_MOUSEOVER] = sprite;
	changeSprite();
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
	ImageAsset img = gameAssets->getImage(archive, id, palette);

	setMouseOverSprite((Image*)img, frame);
}

void Widget::setClickSprite(unsigned button, GuiSprite *sprite) {
	if (button >= MBUTTON_COUNT) {
		throw std::out_of_range("Invalid button ID");
	}

	if (_sprites[WSPRITE_CLICK + button]) {
		delete _sprites[WSPRITE_CLICK + button];
	}

	_sprites[WSPRITE_CLICK + button] = sprite;
	changeSprite();
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
	ImageAsset img = gameAssets->getImage(archive, id, palette);

	setClickSprite(button, (Image*)img, frame);
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

void Widget::redraw(int x, int y, unsigned curtick) {
	if (_cursprite) {
		_cursprite->redraw(x + _x, y + _y, curtick);
	}
}

WidgetContainer::WidgetContainer(void) : _widgets(NULL), _currentWidget(NULL),
	_widgetCount(0), _widgetMax(32) {
	_widgets = new Widget*[_widgetMax];
}

WidgetContainer::~WidgetContainer(void) {
	clearWidgets();
}

Widget *WidgetContainer::createWidget(unsigned x, unsigned y, unsigned width,
	unsigned height) {

	Widget *w = NULL;

	try {
		w = new Widget(x, y, width, height);
		addWidget(w);
	} catch (...) {
		delete w;
		throw;
	}

	return w;
}

void WidgetContainer::addWidget(Widget *w) {
	if (_widgetCount >= _widgetMax) {
		size_t size = _widgetMax ? 2 * _widgetMax : 32;
		Widget **tmp = new Widget*[size];

		if (_widgets) {
			memcpy(tmp, _widgets, _widgetCount * sizeof(Widget*));
			delete[] _widgets;
		}

		_widgets = tmp;
		_widgetMax = size;
	}

	_widgets[_widgetCount++] = w;
}

Widget *WidgetContainer::findWidget(int x, int y) {
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

void WidgetContainer::redrawWidgets(int x, int y, unsigned curtick) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		_widgets[i]->redraw(x, y, curtick);
	}
}

void WidgetContainer::clearWidgets(void) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		delete _widgets[i];
	}

	delete[] _widgets;
	_widgets = NULL;
	_widgetCount = _widgetMax = 0;
}

void WidgetContainer::handleMouseMove(int x, int y, unsigned buttons) {
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

void WidgetContainer::handleMouseDown(int x, int y, unsigned button) {
	if (!_currentWidget) {
		_currentWidget = findWidget(x, y);
	}

	if (_currentWidget) {
		_currentWidget->handleMouseDown(x, y, button);
	}
}

void WidgetContainer::handleMouseUp(int x, int y, unsigned button) {
	if (!_currentWidget) {
		_currentWidget = findWidget(x, y);
	}

	if (_currentWidget) {
		_currentWidget->handleMouseUp(x, y, button);
	}
}

GuiWindow::GuiWindow(GuiView *parent, unsigned flags) : _parent(parent), _x(0),
	_y(0), _grabx(-1), _graby(-1), _width(0), _height(0), _flags(flags) {
	if (_parent) {
		_parent->addWindow(this);
	}
}

GuiWindow::~GuiWindow(void) {
	if (_parent) {
		_parent->removeWindow(this);
	}
}

void GuiWindow::discard(void) {
	if (_parent) {
		_parent->removeWindow(this);
	}

	WidgetContainer::discard();
}

int GuiWindow::isInside(int x, int y) const {
	return x >= _x && x < _x + (int)_width && y >= _y &&
		y < _y + (int)_height;
}

int GuiWindow::isModal(void) const {
	return _flags & WINDOW_MODAL;
}

int GuiWindow::isGrabbed(void) const {
	return _grabx >= 0;
}

void GuiWindow::close(int x, int y, int arg) {
	discard();
}

void GuiWindow::handleMouseMove(int x, int y, unsigned buttons) {
	if ((_flags & WINDOW_MOVABLE) && _grabx >= 0) {
		_x = x - _grabx;
		_y = y - _graby;
		return;
	}

	WidgetContainer::handleMouseMove(x - _x, y - _y, buttons);
}

void GuiWindow::handleMouseDown(int x, int y, unsigned button) {
	x -= _x;
	y -= _y;
	WidgetContainer::handleMouseDown(x, y, button);

	if ((_flags & WINDOW_MOVABLE) && !_currentWidget &&
		button == MBUTTON_LEFT) {
		_grabx = x;
		_graby = y;
	}
}

void GuiWindow::handleMouseUp(int x, int y, unsigned button) {
	WidgetContainer::handleMouseUp(x - _x, y - _y, button);

	if (button == MBUTTON_LEFT) {
		_grabx = _graby = -1;
	}
}

GuiView::GuiView(void) {
	// Initialize window list
	_firstWindow.insert_before(&_lastWindow);
}

GuiView::~GuiView(void) {
	BilistNode<GuiWindow> *next, *ptr = _firstWindow.next();

	// Prevent GuiWindow destructor from discarding list nodes
	_firstWindow.unlink();

	for (; ptr && ptr != &_lastWindow; ptr = next) {
		next = ptr->next();
		delete ptr->data;
		delete ptr;
	}
}

void GuiView::addWindow(GuiWindow *window) {
	_firstWindow.append(window);
}

void GuiView::removeWindow(GuiWindow *window) {
	BilistNode<GuiWindow> *node = _firstWindow.next();

	for (; node && node != &_lastWindow; node = node->next()) {
		if (window != node->data) {
			continue;
		}

		node->discard();
		return;
	}
}

void GuiView::focusWindow(BilistNode<GuiWindow> *node) {
	if (node == _firstWindow.next() || !node->data) {
		return;
	}

	_firstWindow.append(node->data);
	node->discard();
}

BilistNode<GuiWindow> *GuiView::findWindowAt(int x, int y, int ignore_modal) {
	BilistNode<GuiWindow> *node = _firstWindow.next();

	for (; node && node != &_lastWindow; node = node->next()) {
		if (!ignore_modal && node->data->isModal()) {
			// Modal window is blocked unless it's already on top
			if (node == _firstWindow.next() &&
				node->data->isInside(x, y)) {
				return node;
			}

			return NULL;
		}

		if (node->data->isInside(x, y)) {
			return node;
		}
	}

	return NULL;
}

BilistNode<GuiWindow> *GuiView::findModalWindow(void) {
	BilistNode<GuiWindow> *node = _firstWindow.next();

	for (; node && node != &_lastWindow; node = node->next()) {
		if (node->data->isModal()) {
			return node;
		}
	}

	return NULL;
}

void GuiView::redrawWindows(unsigned curtick) {
	BilistNode<GuiWindow> *node = _lastWindow.prev();

	for (; node && node != &_firstWindow; node = node->prev()) {
		node->data->redraw(curtick);
	}
}

void GuiView::exitView(void) {
	gui_stack->remove(this);
}

void GuiView::open(void) {

}

void GuiView::close(void) {

}

void GuiView::handleMouseMove(int x, int y, unsigned buttons) {
	BilistNode<GuiWindow> *node = _firstWindow.next();

	node = (node == &_lastWindow) ? NULL : node;

	if (node && !node->data->isGrabbed()) {
		node = findWindowAt(x, y);
	}

	if (node) {
		if (_currentWidget) {
			_currentWidget->handleMouseOut(x, y, buttons);
		}

		_currentWidget = NULL;
		node->data->handleMouseMove(x, y, buttons);
		return;
	}

	if (findModalWindow()) {
		return;
	}

	WidgetContainer::handleMouseMove(x, y, buttons);
}

void GuiView::handleMouseDown(int x, int y, unsigned button) {
	BilistNode<GuiWindow> *node = findWindowAt(x, y);

	if (node) {
		GuiWindow *w = node->data;

		focusWindow(node);
		w->handleMouseDown(x, y, button);
		return;
	}

	if (findModalWindow()) {
		return;
	}

	WidgetContainer::handleMouseDown(x, y, button);
}

void GuiView::handleMouseUp(int x, int y, unsigned button) {
	BilistNode<GuiWindow> *node = findWindowAt(x, y);

	if (node) {
		node->data->handleMouseUp(x, y, button);
		return;
	}

	if (findModalWindow()) {
		return;
	}

	WidgetContainer::handleMouseUp(x, y, button);
}

TransitionView::TransitionView(Image *background, Image *animation, int x,
	int y) : _background(background), _animation(animation), _x(x), _y(y),
	_startTick(0) {

	gameAssets->takeAsset(_background);
	gameAssets->takeAsset(_animation);
}

TransitionView::~TransitionView(void) {
	gameAssets->freeAsset(_background);
	gameAssets->freeAsset(_animation);
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

ViewStack::ViewStack(void) : _stack(NULL), _top(0), _size(8) {
	_stack = new GuiView*[_size];
	_stack[0] = NULL;
}

ViewStack::~ViewStack(void) {
	clear();
	delete[] _stack;
}

int ViewStack::is_empty(void) const {
	return !_stack[_top];
}

void ViewStack::push(GuiView *view) {
	size_t pos = _top;

	if (_top >= _size - 1) {
		GuiView **tmp, **ptr = new GuiView*[2 * _size];

		memcpy(ptr, _stack, (_top + 1) * sizeof(GuiView*));
		tmp = _stack;
		_stack = ptr;
		_size *= 2;
		delete[] tmp;
	}

	if (_stack[pos]) {
		pos++;
	}

	_stack[pos] = view;
	_top = pos;
}

void ViewStack::pop(void) {
	GuiView *ptr = _stack[_top];

	if (_top > 0) {
		_top--;
	} else {
		_stack[_top] = NULL;
	}

	GarbageCollector::discard(ptr);
}

void ViewStack::remove(GuiView *view) {
	size_t i;

	for (i = 0; i <= _top && _stack[i] != view; i++);

	if (i > _top) {
		return;
	}

	for (; i < _top; i++) {
		_stack[i] = _stack[i + 1];
	}

	if (_top > 0) {
		_top--;
	} else {
		_stack[_top] = NULL;
	}

	GarbageCollector::discard(view);
}

void ViewStack::clear(void) {
	size_t i;

	for (i = 0; i <= _top; i++) {
		GarbageCollector::discard(_stack[i]);
	}

	_stack[0] = NULL;
	_top = 0;
}

GuiView *ViewStack::top(void) {
	GuiView *ret = _stack[_top];

	if (!ret) {
		throw std::runtime_error("View stack is empty");
	}

	return ret;
}
