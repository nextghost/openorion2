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

#define SCROLLBAR_MIN_WIDTH 2
#define SCROLLBAR_MIN_LENGTH 10

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
	_image(NULL), _x(imgx), _y(imgy), _width(0), _height(0), _startTick(0),
	_offsx(offsx), _offsy(offsy), _frame(frame) {

	initImage(img, width, height);
}

GuiSprite::GuiSprite(const char *archive, unsigned id, const uint8_t *palette,
	int offsx, int offsy, int frame, unsigned imgx, unsigned imgy,
	unsigned width, unsigned height) : _image(NULL), _x(imgx), _y(imgy),
	_width(0), _height(0), _startTick(0), _offsx(offsx), _offsy(offsy),
	_frame(frame) {

	ImageAsset img = gameAssets->getImage(archive, id, palette);

	initImage((Image *)img, width, height);
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

void GuiSprite::initImage(Image *img, unsigned width, unsigned height) {
	if (!img) {
		throw std::runtime_error("Image is NULL");
	}

	if (_x >= img->width() || _y >= img->height()) {
		throw std::out_of_range("Subimage offset out of range");
	}

	if (_frame >= 0 && unsigned(_frame) > img->frameCount()) {
		throw std::out_of_range("Image frame out of range");
	}

	gameAssets->takeAsset(img);
	_image = img;
	_width = width ? width : _image->width() - _x;
	_height = height ? height : _image->height() - _y;
}

void GuiSprite::startAnimation(void) {
	_startTick = 0;
}

void GuiSprite::stopAnimation(void) {

}

int GuiSprite::getX(void) const {
	return _offsx;
}

int GuiSprite::getY(void) const {
	return _offsy;
}

unsigned GuiSprite::width(void) const {
	return _width;
}

unsigned GuiSprite::height(void) const {
	return _height;
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
	_disabled(0), _hidden(0), _cursprite(NULL), _disSprite(NULL) {

	memset(_sprites, 0, WIDGET_SPRITES * sizeof(GuiSprite*));
}

Widget::~Widget(void) {
	unsigned i;

	for (i = 0; i < WIDGET_SPRITES; i++) {
		delete _sprites[i];
	}

	delete _disSprite;
}

void Widget::setSprite(unsigned state, GuiSprite *sprite) {
	GuiSprite *oldsprite = _sprites[state];

	_sprites[state] = sprite;
	changeSprite();

	if (oldsprite) {
		oldsprite->discard();
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

unsigned Widget::getX(void) const {
	return _x;
}

unsigned Widget::getY(void) const {
	return _y;
}

unsigned Widget::width(void) const {
	return _width;
}

unsigned Widget::height(void) const {
	return _height;
}

int Widget::isInside(unsigned x, unsigned y) const {
	return isInRect(x, y, _x, _y, _width, _height);
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

void Widget::setDisabledSprite(GuiSprite *sprite) {
	GuiSprite *oldsprite = _disSprite;

	_disSprite = sprite;

	if (oldsprite) {
		if (_disabled) {
			oldsprite->stopAnimation();
		}

		oldsprite->discard();
	}

	if (_disabled && _disSprite) {
		_disSprite->startAnimation();
	}
}

void Widget::setDisabledSprite(Image *img, int frame) {
	GuiSprite *sprite = new GuiSprite(img, 0, 0, frame);

	try {
		setDisabledSprite(sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void Widget::setDisabledSprite(const char *archive, unsigned id,
	const uint8_t *palette, int frame) {
	ImageAsset img = gameAssets->getImage(archive, id, palette);

	setDisabledSprite((Image*)img, frame);
}

void Widget::setIdleSprite(GuiSprite *sprite) {
	setSprite(WSPRITE_IDLE, sprite);
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
	setSprite(WSPRITE_MOUSEOVER, sprite);
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

	setSprite(WSPRITE_CLICK + button, sprite);
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

void Widget::disable(int value) {
	int oldval = _disabled;

	_disabled = value;

	if (_disabled && !oldval) {
		_state = WSTATE_IDLE;
		changeSprite();

		if (_disSprite) {
			if (_cursprite) {
				_cursprite->stopAnimation();
			}

			_disSprite->startAnimation();
		}
	}

	if (!_disabled && oldval) {
		if (_disSprite) {
			_disSprite->stopAnimation();
		}

		changeSprite();
	}
}

void Widget::hide(int value) {
	int oldval = _hidden;

	_hidden = value;

	if (_hidden && !oldval) {
		_state = WSTATE_IDLE;

		if (_cursprite) {
			_cursprite->stopAnimation();
		}
	}

	if (!_hidden && oldval) {
		changeSprite();
	}
}

int Widget::isDisabled(void) const {
	return _disabled;
}

int Widget::isHidden(void) const {
	return _hidden;
}

void Widget::redraw(int x, int y, unsigned curtick) {
	if (_hidden) {
		return;
	}

	if (_disabled && _disSprite) {
		_disSprite->redraw(x + _x, y + _y, curtick);
		return;
	}

	if (_cursprite) {
		_cursprite->redraw(x + _x, y + _y, curtick);
	}
}

ToggleWidget::ToggleWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, Image *img, unsigned offFrame, unsigned onFrame,
	unsigned val) : Widget(x, y, width, height), _valImg(img),
	_offFrame(offFrame), _onFrame(onFrame), _value(val) {

	unsigned fcount;

	if (!img) {
		throw std::invalid_argument("On/off image is required");
	}

	fcount = img->frameCount();

	if (_offFrame >= fcount || _onFrame >= fcount) {
		throw std::invalid_argument("Invalid on/off frame ID");
	}

	setValue(_value);
	gameAssets->takeAsset(_valImg);
}

ToggleWidget::ToggleWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, const char *archive, unsigned id,
	const uint8_t *palette, unsigned offFrame, unsigned onFrame,
	unsigned val) : Widget(x, y, width, height), _valImg(NULL),
	_offFrame(offFrame), _onFrame(onFrame), _value(val) {

	ImageAsset img = gameAssets->getImage(archive, id, palette);
	unsigned fcount = img->frameCount();

	if (_offFrame >= fcount || _onFrame >= fcount) {
		throw std::invalid_argument("Invalid on/off frame ID");
	}

	_valImg = (Image*)img;
	setValue(_value);
	gameAssets->takeAsset(_valImg);
}

ToggleWidget::~ToggleWidget(void) {
	gameAssets->freeAsset(_valImg);
}

void ToggleWidget::setValue(unsigned val) {
	_value = !!val;
	setIdleSprite(_valImg, _value ? _onFrame : _offFrame);
}

unsigned ToggleWidget::value(void) const {
	return _value;
}

void ToggleWidget::handleMouseUp(int x, int y, unsigned button) {
	if (button == MBUTTON_LEFT) {
		setValue(!_value);
	}

	Widget::handleMouseUp(x, y, button);
}

ChoiceWidget::ChoiceWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, unsigned choiceCount) : Widget(x, y, width, height),
		_count(choiceCount), _value(0), _valSprites(NULL),
		_buttons(NULL), _curButton(NULL) {

	if (choiceCount < 1) {
		throw std::out_of_range("ChoiceWidget needs at least 1 choice");
	}

	_valSprites = new GuiSprite*[_count];
	memset(_valSprites, 0, _count * sizeof(GuiSprite*));

	try {
		_buttons = new Widget*[_count];
	} catch (...) {
		delete[] _valSprites;
		throw;
	}

	memset(_buttons, 0, _count * sizeof(Widget*));
}

ChoiceWidget::~ChoiceWidget(void) {
	unsigned i;

	for (i = 0; i < _count; i++) {
		delete _valSprites[i];
		delete _buttons[i];
	}

	delete[] _valSprites;
	delete[] _buttons;
}

int ChoiceWidget::findChoice(int x, int y) {
	unsigned i;

	for (i = 0; i < _count; i++) {
		if (_buttons[i] && _buttons[i]->isInside(x, y)) {
			return i;
		}
	}

	return -1;
}

Widget *ChoiceWidget::findButton(int x, int y) {
	int choice = findChoice(x, y);

	return choice >= 0 ? _buttons[choice] : NULL;
}

void ChoiceWidget::setValueChangeCallback(const GuiCallback &callback) {
	_onChange = callback;
}

void ChoiceWidget::setChoiceButton(unsigned val, unsigned x, unsigned y,
	unsigned width, unsigned height, GuiSprite *sprite) {

	if (val >= _count) {
		throw std::out_of_range("Invalid choice button");
	}

	if (_buttons[val]) {
		throw std::logic_error("Choice button is already defined");
	}

	_buttons[val] = new Widget(x, y, width, height);
	_valSprites[val] = sprite;
}

void ChoiceWidget::setChoiceButton(unsigned val, unsigned x, unsigned y,
	unsigned width, unsigned height, Image *img, int frame) {

	GuiSprite *sprite = new GuiSprite(img, x, y, frame);

	try {
		setChoiceButton(val, x, y, width, height, sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void ChoiceWidget::setChoiceButton(unsigned val, unsigned x, unsigned y,
	unsigned width, unsigned height, const char *archive, unsigned id,
	const uint8_t *palette, int frame) {

	ImageAsset img = gameAssets->getImage(archive, id, palette);

	setChoiceButton(val, x, y, width, height, (Image*)img, frame);
}

Widget *ChoiceWidget::button(unsigned id) {
	if (id >= _count) {
		throw std::out_of_range("Invalid button ID");
	}

	return _buttons[id];
}

void ChoiceWidget::setValue(unsigned val) {
	if (val >= _count) {
		throw std::out_of_range("Invalid choice value");
	}

	_value = val;
}

unsigned ChoiceWidget::value(void) const {
	return _value;
}

void ChoiceWidget::handleMouseMove(int x, int y, unsigned buttons) {
	int dx = x - getX(), dy = x - getY();
	Widget *w = findButton(dx, dy);

	if (w != _curButton) {
		if (_curButton) {
			_curButton->handleMouseOut(dx, dy, buttons);
		}

		if (w) {
			w->handleMouseOver(dx, dy, buttons);
		}

		_curButton = w;
	}

	if (w) {
		w->handleMouseMove(dx, dy, buttons);
	}

	Widget::handleMouseMove(x, y, buttons);
}

void ChoiceWidget::handleMouseOut(int x, int y, unsigned buttons) {
	if (_curButton) {
		_curButton->handleMouseOut(x - getX(), y - getX(), buttons);
	}

	_curButton = NULL;
	Widget::handleMouseOut(x, y, buttons);
}

void ChoiceWidget::handleMouseDown(int x, int y, unsigned button) {
	int dx = x - getX(), dy = x - getY();
	Widget *w = findButton(dx, dy);

	if (w) {
		w->handleMouseDown(dx, dy, button);
	}

	Widget::handleMouseDown(x, y, button);
}

void ChoiceWidget::handleMouseUp(int x, int y, unsigned button) {
	int val, dx = x - getX(), dy = y - getY();

	val = findChoice(dx, dy);

	if (val >= 0) {
		if (button == MBUTTON_LEFT && _value != (unsigned)val) {
			_value = val;
			_onChange(x, y);
		}

		_buttons[val]->handleMouseUp(dx, dy, button);
	}

	Widget::handleMouseUp(x, y, button);
}

void ChoiceWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i;

	if (_valSprites[_value]) {
		_valSprites[_value]->redraw(x + getX(), y + getY(), curtick);
	}

	for (i = 0; i < _count; i++) {
		if (_buttons[i]) {
			_buttons[i]->redraw(x + getX(), y + getY(), curtick);
		}
	}
}

ScrollBarWidget::ScrollBarWidget(unsigned x, unsigned y, unsigned width,
	unsigned height, unsigned pagesize, unsigned range,
	const uint8_t *texture) : Widget(x, y, width, height), _position(0),
	_step(1), _pagesize(pagesize), _range(range), _slidePos(0),
	_slideLength(10), _grabOffset(-1), _texture(NULL) {

	unsigned size = 3 * (width < height ? width : height);

	if (width < SCROLLBAR_MIN_WIDTH || height < SCROLLBAR_MIN_WIDTH) {
		throw std::invalid_argument("Scrollbar width is too narrow");
	}

	if (width < SCROLLBAR_MIN_LENGTH && height < SCROLLBAR_MIN_LENGTH) {
		throw std::invalid_argument("Scrollbar length is too short");
	}

	if (!pagesize) {
		throw std::invalid_argument("Page size must not be zero");
	}

	_texture = new uint8_t[size];
	memcpy(_texture, texture, size * sizeof(uint8_t));
	setRange(range);
}

ScrollBarWidget::~ScrollBarWidget(void) {
	delete[] _texture;
}

unsigned ScrollBarWidget::slideWidth(void) const {
	unsigned w = width(), h = height();

	return w < h ? w : h;
}

unsigned ScrollBarWidget::pixelRange(void) const {
	unsigned w = width(), h = height();

	return w < h ? h : w;
}

void ScrollBarWidget::updateSlide(void) {
	unsigned pos = _position * (pixelRange() - _slideLength);

	_slidePos = _range > 0 ? pos / _range : 0;
}

void ScrollBarWidget::setPosition(unsigned position) {
	if (position >= _range + _pagesize) {
		throw std::out_of_range("Position is out of range");
	}

	_position = position > _range ? _range : position;
	updateSlide();
}

void ScrollBarWidget::setStep(unsigned step) {
	if (!step) {
		throw std::invalid_argument("Step must not be zero");
	}

	_step = step;
}

void ScrollBarWidget::setRange(unsigned range) {
	unsigned tmp;

	_position = _slidePos = 0;
	_range = range > _pagesize ? range - _pagesize : 0;
	tmp = pixelRange() / (_range + 1);
	_slideLength = tmp < SCROLLBAR_MIN_LENGTH ? SCROLLBAR_MIN_LENGTH : tmp;
	_onScroll(0, 0);
}

unsigned ScrollBarWidget::position(void) const {
	return _position;
}

unsigned ScrollBarWidget::step(void) const {
	return _step;
}

unsigned ScrollBarWidget::pagesize(void) const {
	return _pagesize;
}

unsigned ScrollBarWidget::range(void) const {
	return _range + _pagesize;
}

void ScrollBarWidget::setBeginScrollCallback(const GuiCallback &callback) {
	_onBeginScroll = callback;
}

void ScrollBarWidget::setScrollCallback(const GuiCallback &callback) {
	_onScroll = callback;
}

void ScrollBarWidget::setEndScrollCallback(const GuiCallback &callback) {
	_onEndScroll = callback;
}

void ScrollBarWidget::handleMouseMove(int x, int y, unsigned buttons) {
	unsigned w, h;
	int pos, len;

	// User released left mouse button outside the scrollbar,
	// release slider grab
	if (_grabOffset >= 0 && !(buttons & (1 << MBUTTON_LEFT))) {
		handleMouseUp(x, y, MBUTTON_LEFT);
	}

	if (_grabOffset >= 0) {
		// Note: x and y may be outside the scroll bar widget here
		w = width();
		h = height();
		len = pixelRange() - _slideLength;
		pos = w < h ? y - getY() : x - getX();
		pos = pos < _grabOffset ? 0 : pos - _grabOffset;
		pos = pos > len ? len : pos;
		_slidePos = pos;
		pos += _range > 0 ? len / (2 * _range) : 0;
		pos = len > 0 ? (pos * _range) / len : 0;
		_position = unsigned(pos) > _range ? _range : pos;
		_onScroll(x, y);
	}

	Widget::handleMouseMove(x, y, buttons);
}

void ScrollBarWidget::handleMouseDown(int x, int y, unsigned button) {
	unsigned len, w = width(), h = height();
	unsigned pos = w < h ? y - getY() : x - getX();

	if (button != MBUTTON_LEFT) {
		Widget::handleMouseDown(x, y, button);
		return;
	}

	// clicked on slider, grab it
	if (pos >= _slidePos && pos < _slidePos + _slideLength) {
		_grabOffset = pos - _slidePos;
		_onBeginScroll(x, y);
		return;
	}

	// clicked outside the slider, jump to that position
	len = pixelRange() - _slideLength;
	pos = len > 0 ? (pos * _range) / len : 0;
	_position = pos > _range ? _range : pos;
	updateSlide();
	_onScroll(x, y);
}

void ScrollBarWidget::handleMouseUp(int x, int y, unsigned button) {
	if (button != MBUTTON_LEFT || _grabOffset < 0) {
		Widget::handleMouseUp(x, y, button);
		return;
	}

	_grabOffset = -1;
	updateSlide();	// snap slider back to discrete position
	_onEndScroll(x, y);
	// _position did not change, no need to call _onScroll()
}

void ScrollBarWidget::scrollMinus(int x, int y, int arg) {
	_position = _position > _step ? _position - _step : 0;
	updateSlide();
	_onScroll(x, y);
}

void ScrollBarWidget::scrollPlus(int x, int y, int arg) {
	unsigned pos = _position + _step;

	_position = pos > _range ? _range : pos;
	updateSlide();
	_onScroll(x, y);
}

void ScrollBarWidget::redraw(int x, int y, unsigned curtick) {
	unsigned i, thickness, dx, dy, swidth, sheight, ewidth, eheight;
	const uint8_t *color = _texture;

	x += getX();
	y += getY();

	if (width() < height()) {
		thickness = width();
		dx = swidth = eheight = 1;
		dy = 0;
		sheight = _slideLength;
		ewidth = thickness - 2;
		y += _slidePos;
	} else {
		thickness = height();
		dx = 0;
		dy = sheight = ewidth = 1;
		swidth = _slideLength;
		eheight = thickness - 2;
		x += _slidePos;
	}

	for (i = 0; i < thickness; i++, color += 3) {
		drawRect(x + i * dx, y + i * dy, swidth, sheight, color[0],
			color[1], color[2]);
	}

	color -= 3;
	drawRect(x + dx, y + dy, ewidth, eheight, _texture[0], _texture[1],
		_texture[2]);
	drawRect(x + dx + dy * (_slideLength - 1),
		y + dy + dx * (_slideLength - 1), ewidth, eheight,
		color[0], color[1], color[2]);
}

WidgetContainer::WidgetContainer(void) : _widgets(NULL), _widgetCount(0),
	_widgetMax(32), _currentWidget(NULL) {
	_widgets = new Widget*[_widgetMax];
}

WidgetContainer::~WidgetContainer(void) {
	clearWidgets();
}

Widget *WidgetContainer::createWidget(unsigned x, unsigned y, unsigned width,
	unsigned height) {

	Widget *w = new Widget(x, y, width, height);

	addWidget(w);
	return w;
}

void WidgetContainer::addWidget(Widget *w) {
	if (_widgetCount >= _widgetMax) {
		size_t size = _widgetMax ? 2 * _widgetMax : 32;
		Widget **tmp;

		try {
			tmp = new Widget*[size];
		} catch (...) {
			delete w;
			throw;
		}

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
		if (!(*ptr)->isHidden() && (*ptr)->isInside(x, y)) {
			return (*ptr)->isDisabled() ? NULL : *ptr;
		}
	}

	return NULL;
}

void WidgetContainer::redrawWidgets(int x, int y, unsigned curtick) {
	size_t i;

	for (i = 0; i < _widgetCount; i++) {
		if (!_widgets[i]->isHidden()) {
			_widgets[i]->redraw(x, y, curtick);
		}
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
	return isInRect(x, y, _x, _y, _width, _height);
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

TextLayout::TextLayout(void) : _blocks(NULL), _sprites(NULL), _height(0),
	_blockCount(0), _blockSize(0), _spriteCount(0), _spriteSize(0) {

}

TextLayout::~TextLayout(void) {
	unsigned i;

	for (i = 0; i < _blockCount; i++) {
		delete[] _blocks[i].text;
	}

	for (i = 0; i < _spriteCount; i++) {
		delete _sprites[i];
	}

	delete[] _blocks;
	delete[] _sprites;
}

TextLayout::TextBlock *TextLayout::addBlock(const char *text, ssize_t len) {
	TextBlock *ret;

	if (_blockCount >= _blockSize) {
		TextBlock *tmp, *ptr;
		unsigned newsize = _blockSize ? 2 * _blockSize : 8;

		ptr = new TextBlock[newsize];

		if (_blockCount) {
			memcpy(ptr, _blocks, _blockCount * sizeof(TextBlock));
		}

		tmp = _blocks;
		_blocks = ptr;
		_blockSize = newsize;
		delete[] tmp;
	}

	ret = _blocks + _blockCount;

	if (len < 0) {
		ret->text = copystr(text);
	} else {
		ret->text = new char[len+1];
		strncpy(ret->text, text, len);
		ret->text[len] = '\0';
	}

	_blockCount++;
	return ret;
}

void TextLayout::adjustLine(unsigned lineStart, unsigned align,
	unsigned maxwidth) {
	unsigned i, width, extra;
	TextBlock *ptr;

	if (lineStart >= _blockCount) {
		return;
	}

	ptr = _blocks + _blockCount - 1;
	width = maxwidth - (ptr->x + ptr->width);

	switch (align) {
	case ALIGN_CENTER:
		width /= 2;
		// fall through

	case ALIGN_RIGHT:
		for (i = lineStart; i < _blockCount; i++) {
			_blocks[i].x += width;
		}

		break;

	case ALIGN_JUSTIFY:
		i = _blockCount - lineStart - 1;

		if (i < 1) {
			return;
		}

		extra = width % i;
		width /= i;

		for (i = 1; lineStart + i < _blockCount; i++) {
			_blocks[lineStart + i].x += i * width;
			_blocks[lineStart + i].x += i < extra ? i : extra;
		}

		break;
	}
}

void TextLayout::appendText(const char *text, unsigned x, unsigned y,
	unsigned maxwidth, unsigned font, unsigned color, unsigned outline,
	unsigned align) {

	unsigned i = 0, tmp, wordStart, width, curx = 0, lineStart = 0;
	unsigned line_height;
	int ret;
	char c;
	TextBlock *blk;
	Font *fnt = gameFonts->getFont(font);

	line_height = fnt->height() + 1;

	while (text[i]) {
		for (; text[i] && text[i] <= ' '; i++) {
			switch (text[i]) {
			case '\n':
				if (align != ALIGN_JUSTIFY) {
					adjustLine(lineStart, align, maxwidth);
				}

				lineStart = _blockCount;
				curx = 0;
				y += line_height;
				break;

			// Absolute text position
			case 0x7:
				ret = sscanf(text + i + 1, "%c%u", &c, &tmp);

				if (ret >= 2 && c == 'X') {
					for (; text[i] && text[i] != '.'; i++);
					curx = tmp;
					lineStart = _blockCount;
				}

				break;

			default:
				curx += fnt->charWidth(text[i]) + 1;
				break;
			}
		}

		if (!text[i]) {
			break;
		}

		for (width = 0, wordStart = i; text[i] > ' '; i++) {
			width += fnt->charWidth(text[i]) + 1;
		}

		if (curx + width > maxwidth + 1 && lineStart < _blockCount) {
			adjustLine(lineStart, align, maxwidth);
			lineStart = _blockCount;
			curx = 0;
			y += line_height;
		}

		blk = addBlock(text + wordStart, i - wordStart);
		blk->x = curx;
		blk->y = y;
		blk->width = width - 1;
		blk->font = font;
		blk->color = color;
		blk->outline = outline;
		curx += width;
	}

	if (align != ALIGN_JUSTIFY) {
		adjustLine(lineStart, align, maxwidth);
	}

	if (_blockCount) {
		tmp = _blocks[_blockCount - 1].y + line_height;
		_height = _height < tmp ? tmp : _height;
	}

	// keep trailing newlines
	_height = _height < y ? y : _height;
}

void TextLayout::addSprite(GuiSprite *sprite) {
	int bottom;

	if (_spriteCount >= _spriteSize) {
		GuiSprite **tmp, **ptr;
		unsigned newsize = _spriteSize ? 2 * _spriteSize : 8;

		ptr = new GuiSprite*[newsize];

		if (_spriteCount) {
			memcpy(ptr, _sprites, _spriteCount*sizeof(GuiSprite*));
		}

		tmp = _sprites;
		_sprites = ptr;
		_spriteSize = newsize;
		delete[] tmp;
	}

	_sprites[_spriteCount] = sprite;
	_spriteCount++;
	bottom = sprite->getY() + sprite->height();

	if (bottom > 0 && _height < (unsigned)bottom) {
		_height = bottom;
	}

	sprite->startAnimation();
}

void TextLayout::addSprite(unsigned x, unsigned y, Image *img, int frame) {
	GuiSprite *sprite = new GuiSprite(img, x, y, frame);

	try {
		addSprite(sprite);
	} catch (...) {
		delete sprite;
		throw;
	}
}

void TextLayout::addSprite(unsigned x, unsigned y, const char *archive,
	unsigned id, const uint8_t *palette, int frame) {

	ImageAsset img = gameAssets->getImage(archive, id, palette);

	addSprite(x, y, (Image*)img, frame);
}

void TextLayout::redraw(unsigned x, unsigned y, unsigned curtick) {
	unsigned i;
	TextBlock *block;
	Font *fnt;

	for (i = 0; i < _spriteCount; i++) {
		_sprites[i]->redraw(x, y, curtick);
	}

	for (i = 0, block = _blocks; i < _blockCount; i++, block++) {
		if (!i || _blocks[i-1].font != _blocks[i].font) {
			fnt = gameFonts->getFont(block->font);
		}

		fnt->renderText(x + block->x, y + block->y, block->color,
			block->text, block->outline);
	}
}

unsigned TextLayout::height(void) const {
	return _height;
}
