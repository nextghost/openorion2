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

#define ZSTATE_IDLE 0
#define ZSTATE_MOUSEOVER 1
#define ZSTATE_LEFTCLICK 2
#define ZSTATE_RIGHTCLICK 4

#define MBUTTON_LEFT 0
#define MBUTTON_RIGHT 1
#define MBUTTON_OTHER 2

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

class ActiveZone {
public:
	unsigned x, y, width, height, state;
	GuiCallback onMouseOver, onMouseOut, onMouseMove, onMouseDown;
	GuiCallback onMouseUp, onMouseRightDown, onMouseRightUp;

	explicit ActiveZone(unsigned left = 0, unsigned top = 0,
		unsigned w = 0, unsigned h = 0);
};

class GuiView {
private:
	ActiveZone **_zones, *_currentZone;
	size_t _zoneCount, _zoneMax;

	// Do NOT implement
	GuiView(const GuiView &other);
	const GuiView &operator=(const GuiView &other);

protected:
	void addZone(ActiveZone *zone);
	ActiveZone *findZone(int x, int y);

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

#endif
