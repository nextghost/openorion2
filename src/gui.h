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

#ifndef GUI_H_
#define GUI_H_

#include "gfx.h"

#define MBUTTON_LEFT 0
#define MBUTTON_RIGHT 1
#define MBUTTON_OTHER 2
#define MBUTTON_COUNT 3

#define WIDGET_SPRITES (MBUTTON_COUNT + 2)

#define ANIM_LOOP -1	// play the animation in a loop
#define ANIM_ONCE -2	// hide the animation after playing once
#define ANIM_STICKY -3	// play the animation once and freeze on last frame

class GuiCallback {
private:
	GuiCallback *_callback;

protected:
	virtual GuiCallback *copy(void) const;

public:
	GuiCallback(void);
	GuiCallback(const GuiCallback &other);
	virtual ~GuiCallback(void);

	const GuiCallback &operator=(const GuiCallback &other);
	virtual void operator()(int x, int y);
};

template <class C> class GuiMethodCallback : public GuiCallback {
private:
	C *_instance;
	void (C::*_method)(int, int, int);
	int _arg;

protected:
	GuiCallback *copy(void) const;

public:
	GuiMethodCallback(C &instance, void (C::*method)(int, int, int),
		int arg = 0);
	GuiMethodCallback(const GuiMethodCallback &other);

	void operator()(int x, int y);
};

class GuiSprite {
private:
	Image *_image;
	unsigned _x, _y, _width, _height, _startTick;
	int _offsx, _offsy, _frame;

public:
	explicit GuiSprite(Image *img, int offsx = 0, int offsy = 0,
		int frame = ANIM_LOOP, unsigned imgx = 0, unsigned imgy = 0,
		unsigned width = 0, unsigned height = 0);
	GuiSprite(const GuiSprite &other);
	virtual ~GuiSprite(void);

	const GuiSprite &operator=(const GuiSprite &other);

	virtual void startAnimation(void);
	virtual void stopAnimation(void);

	// Draw subimage (_x, _y, _width, _height) at (x+_offsx, y+_offsy)
	virtual void redraw(unsigned x, unsigned y, unsigned curtick);
};

class Widget {
private:
	unsigned _x, _y, _width, _height, _state;
	GuiCallback _onMouseOver, _onMouseOut, _onMouseMove;
	GuiCallback _onMouseDown[MBUTTON_COUNT], _onMouseUp[MBUTTON_COUNT];
	GuiSprite *_cursprite, *_sprites[WIDGET_SPRITES];

	// Do NOT implement
	Widget(const Widget &other);
	const Widget &operator=(const Widget &other);

protected:
	virtual void changeSprite(void);

public:
	Widget(unsigned x, unsigned y, unsigned width, unsigned height);
	virtual ~Widget(void);

	virtual int isInside(unsigned x, unsigned y) const;

	virtual void setMouseOverCallback(const GuiCallback &callback);
	virtual void setMouseMoveCallback(const GuiCallback &callback);
	virtual void setMouseOutCallback(const GuiCallback &callback);
	virtual void setMouseDownCallback(unsigned button,
		const GuiCallback &callback);
	virtual void setMouseUpCallback(unsigned button,
		const GuiCallback &callback);

	virtual void setIdleSprite(GuiSprite *sprite);
	virtual void setIdleSprite(Image *img, int frame = ANIM_LOOP);
	virtual void setIdleSprite(const char *archive, unsigned id,
		const uint8_t *palette = NULL, int frame = ANIM_LOOP);
	virtual void setMouseOverSprite(GuiSprite *sprite);
	virtual void setMouseOverSprite(Image *img, int frame = ANIM_LOOP);
	virtual void setMouseOverSprite(const char *archive, unsigned id,
		const uint8_t *palette = NULL, int frame = ANIM_LOOP);
	virtual void setClickSprite(unsigned button, GuiSprite *sprite);
	virtual void setClickSprite(unsigned button, Image *img,
		int frame = ANIM_LOOP);
	virtual void setClickSprite(unsigned button, const char *archive,
		unsigned id, const uint8_t *palette = NULL,
		int frame = ANIM_LOOP);

	virtual void handleMouseOver(int x, int y, unsigned buttons);
	virtual void handleMouseMove(int x, int y, unsigned buttons);
	virtual void handleMouseOut(int x, int y, unsigned buttons);
	virtual void handleMouseDown(int x, int y, unsigned button);
	virtual void handleMouseUp(int x, int y, unsigned button);

	virtual void redraw(unsigned curtick);
};

class GuiView {
private:
	Widget **_widgets, *_currentWidget;
	size_t _widgetCount, _widgetMax;

	// Do NOT implement
	GuiView(const GuiView &other);
	const GuiView &operator=(const GuiView &other);

protected:
	void addWidget(Widget *w);
	Widget *findWidget(int x, int y);
	void redrawWidgets(unsigned curtick);
	void clearWidgets(void);

	// Discard this instance from view stack and switch to the next view
	// (if any). It is safe to access instance variable after calling
	// this method. The instance will be garbage collected after control
	// returns to the main loop.
	void exitView(void);

public:
	GuiView(void);
	virtual ~GuiView(void);

	virtual void redraw(unsigned curtick) = 0;

	// methods called on view transitions (may be called multiple times
	// on the same instance)
	virtual void open(void);
	virtual void close(void);

	virtual void handleMouseMove(int x, int y, unsigned buttons);
	virtual void handleMouseDown(int x, int y, unsigned button);
	virtual void handleMouseUp(int x, int y, unsigned button);
};

// Simple skippable view transition animation
// TODO: Add support for transition audio
class TransitionView : public GuiView {
private:
	Image *_background, *_animation;
	int _x, _y;
	unsigned _startTick;

public:
	TransitionView(Image *background, Image *animation, int x = 0,
		int y = 0);
	~TransitionView(void);

	void redraw(unsigned curtick);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseDown(int x, int y, unsigned button);
	void handleMouseUp(int x, int y, unsigned button);
};

class ViewStack {
private:
	GuiView **_stack, **_garbage;
	size_t _top, _size, _garbage_size, _garbage_count;

	// Do NOT implement
	ViewStack(const ViewStack &other);
	const ViewStack &operator=(const ViewStack &other);

protected:
	void expand_garbage_bin(size_t size = 0);

public:
	ViewStack(void);
	~ViewStack(void);

	int is_empty(void) const;

	// Add new view on top of the stack. Adding the same view instance more
	// than once is not allowed.
	// Only one thread may modify the stack at a time.
	void push(GuiView *view);
	void pop(void);
	void remove(GuiView *view);

	// move all current views to garbage
	void clear(void);

	// collect garbage
	void flush(void);

	// Returns the current view. Thread safe.
	GuiView *top(void);
};

// Helper function for creating GuiMethodCallbacks with type inference
template <class C>
GuiCallback GuiMethod(C &instance, void (C::*method)(int,int,int), int arg=0) {
	return GuiMethodCallback<C>(instance, method, arg);
}

template <class C>
GuiMethodCallback<C>::GuiMethodCallback(C &instance,
	void (C::*method)(int,int,int), int arg) : _instance(&instance),
	_method(method), _arg(arg) {

}

template <class C>
GuiMethodCallback<C>::GuiMethodCallback(const GuiMethodCallback &other) :
	GuiCallback(), _instance(other._instance), _method(other._method),
	_arg(other._arg) {

}

template <class C>
void GuiMethodCallback<C>::operator()(int x, int y) {
	(_instance->*_method)(x, y, _arg);
}

template <class C>
GuiCallback *GuiMethodCallback<C>::copy(void) const {
	return new GuiMethodCallback<C>(*this);
}


extern ViewStack *gui_stack;

#endif
