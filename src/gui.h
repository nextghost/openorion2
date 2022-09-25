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
#include "utils.h"

#define MBUTTON_LEFT 0
#define MBUTTON_RIGHT 1
#define MBUTTON_OTHER 2
#define MBUTTON_COUNT 3

#define WIDGET_SPRITES (MBUTTON_COUNT + 2)

#define ANIM_LOOP -1	// play the animation in a loop
#define ANIM_ONCE -2	// hide the animation after playing once
#define ANIM_STICKY -3	// play the animation once and freeze on last frame

#define WINDOW_MODAL 1
#define WINDOW_MOVABLE 2

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2
#define ALIGN_JUSTIFY 3

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

class GuiSprite : public Recyclable {
private:
	Image *_image;
	unsigned _x, _y, _width, _height, _startTick;
	int _offsx, _offsy, _frame;

protected:
	void initImage(Image *img, unsigned width, unsigned height);

public:
	explicit GuiSprite(Image *img, int offsx = 0, int offsy = 0,
		int frame = ANIM_LOOP, unsigned imgx = 0, unsigned imgy = 0,
		unsigned width = 0, unsigned height = 0);
	GuiSprite(const char *archive, unsigned id,
		const uint8_t *palette = NULL, int offsx = 0, int offsy = 0,
		int frame = ANIM_LOOP, unsigned imgx = 0, unsigned imgy = 0,
		unsigned width = 0, unsigned height = 0);
	GuiSprite(const GuiSprite &other);
	virtual ~GuiSprite(void);

	const GuiSprite &operator=(const GuiSprite &other);

	virtual void startAnimation(void);
	virtual void stopAnimation(void);

	int getX(void) const;
	int getY(void) const;
	unsigned width(void) const;
	unsigned height(void) const;

	// Draw subimage (_x, _y, _width, _height) at (x+_offsx, y+_offsy)
	virtual void redraw(unsigned x, unsigned y, unsigned curtick);
};

class TextLayout : public Recyclable {
private:
	struct TextBlock {
		unsigned x, y, width, font, color, outline;
		char *text;
	};

	TextBlock *_blocks;
	GuiSprite **_sprites;
	unsigned _height, _blockCount, _blockSize, _spriteCount, _spriteSize;

	// Do NOT implement
	TextLayout(const TextLayout &other);
	const TextLayout &operator=(const TextLayout &other);

protected:
	TextBlock *addBlock(const char *text, ssize_t len = -1);
	void adjustLine(unsigned lineStart, unsigned align, unsigned maxwidth);

public:
	TextLayout(void);
	~TextLayout(void);

	void appendText(const char *text, unsigned x, unsigned y,
		unsigned maxwidth, unsigned font, unsigned color,
		unsigned outline = OUTLINE_NONE, unsigned align = ALIGN_LEFT);
	void addSprite(GuiSprite *sprite);
	void addSprite(unsigned x, unsigned y, Image *img,
		int frame = ANIM_LOOP);
	void addSprite(unsigned x, unsigned y, const char *archive,
		unsigned id, const uint8_t *palette = NULL,
		int frame = ANIM_LOOP);

	void redraw(unsigned x, unsigned y, unsigned curtick);

	unsigned height(void) const;
};

class Widget {
private:
	unsigned _x, _y, _width, _height, _state;
	int _disabled, _hidden;
	GuiCallback _onMouseOver, _onMouseOut, _onMouseMove;
	GuiCallback _onMouseDown[MBUTTON_COUNT], _onMouseUp[MBUTTON_COUNT];
	GuiSprite *_cursprite, *_disSprite, *_sprites[WIDGET_SPRITES];

	// Do NOT implement
	Widget(const Widget &other);
	const Widget &operator=(const Widget &other);

protected:
	virtual void setSprite(unsigned state, GuiSprite *sprite);
	virtual void changeSprite(void);

public:
	Widget(unsigned x, unsigned y, unsigned width, unsigned height);
	virtual ~Widget(void);

	unsigned getX(void) const;
	unsigned getY(void) const;
	unsigned width(void) const;
	unsigned height(void) const;
	virtual int isInside(unsigned x, unsigned y) const;

	virtual void setMouseOverCallback(const GuiCallback &callback);
	virtual void setMouseMoveCallback(const GuiCallback &callback);
	virtual void setMouseOutCallback(const GuiCallback &callback);
	virtual void setMouseDownCallback(unsigned button,
		const GuiCallback &callback);
	virtual void setMouseUpCallback(unsigned button,
		const GuiCallback &callback);

	virtual void setDisabledSprite(GuiSprite *sprite);
	virtual void setDisabledSprite(Image *img, int frame = ANIM_LOOP);
	virtual void setDisabledSprite(const char *archive, unsigned id,
		const uint8_t *palette = NULL, int frame = ANIM_LOOP);
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

	virtual void disable(int value);
	virtual void hide(int value);
	int isDisabled(void) const;
	int isHidden(void) const;

	virtual void redraw(int x, int y, unsigned curtick);
};

class ToggleWidget : public Widget {
private:
	Image *_valImg;
	unsigned _offFrame, _onFrame, _value;

public:
	ToggleWidget(unsigned x, unsigned y, unsigned width, unsigned height,
		Image *img, unsigned offFrame = 0, unsigned onFrame = 1,
		unsigned val = 0);
	ToggleWidget(unsigned x, unsigned y, unsigned width, unsigned height,
		const char *archive, unsigned id,
		const uint8_t *palette = NULL, unsigned offFrame = 0,
		unsigned onFrame = 1, unsigned val = 0);
	~ToggleWidget(void);

	void setValue(unsigned val);
	unsigned value(void) const;

	void handleMouseUp(int x, int y, unsigned button);
};

class ChoiceWidget : public Widget {
private:
	unsigned _count, _value;
	GuiCallback _onChange;
	GuiSprite **_valSprites;
	Widget **_buttons, *_curButton;

protected:
	int findChoice(int x, int y);
	Widget *findButton(int x, int y);

public:
	ChoiceWidget(unsigned x, unsigned y, unsigned width, unsigned height,
		unsigned choiceCount);
	~ChoiceWidget(void);

	void setValueChangeCallback(const GuiCallback &callback);

	virtual void setChoiceButton(unsigned val, unsigned x, unsigned y,
		unsigned width, unsigned height, GuiSprite *sprite);
	virtual void setChoiceButton(unsigned val, unsigned x, unsigned y,
		unsigned width, unsigned height, Image *img,
		int frame = ANIM_LOOP);
	virtual void setChoiceButton(unsigned val, unsigned x, unsigned y,
		unsigned width, unsigned height, const char *archive,
		unsigned id, const uint8_t *palette = NULL,
		int frame = ANIM_LOOP);
	virtual Widget *button(unsigned id);

	void setValue(unsigned val);
	unsigned value(void) const;

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseOut(int x, int y, unsigned buttons);
	void handleMouseDown(int x, int y, unsigned button);
	void handleMouseUp(int x, int y, unsigned button);

	void redraw(int x, int y, unsigned curtick);
};

class ScrollBarWidget : public Widget {
private:
	GuiCallback _onBeginScroll, _onScroll, _onEndScroll;
	unsigned _position, _step, _pagesize, _range;
	unsigned _slidePos, _slideLength;	// rendering info
	int _grabOffset;	// mouse grab position on slider
	uint8_t *_texture;

protected:
	unsigned slideWidth(void) const;
	unsigned pixelRange(void) const;
	void updateSlide(void);

public:
	// Slider will be MIN(width, height) pixels wide and texture must
	// contain that many RGB triplets. If width < height, scroll bar
	// will be vertical, otherwise it'll be horizontal.
	ScrollBarWidget(unsigned x, unsigned y, unsigned width,
		unsigned height, unsigned pagesize, unsigned range,
		const uint8_t *texture);
	~ScrollBarWidget(void);

	void setPosition(unsigned position);
	void setStep(unsigned step);
	void setRange(unsigned range);
	unsigned position(void) const;
	unsigned step(void) const;
	unsigned pagesize(void) const;
	unsigned range(void) const;

	void setBeginScrollCallback(const GuiCallback &callback);
	void setScrollCallback(const GuiCallback &callback);
	void setEndScrollCallback(const GuiCallback &callback);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseDown(int x, int y, unsigned button);
	void handleMouseUp(int x, int y, unsigned button);

	void scrollMinus(int x, int y, int arg);
	void scrollPlus(int x, int y, int arg);

	void redraw(int x, int y, unsigned curtick);
};

class LabelWidget : public Widget {
private:
	TextLayout *_layout;
	char *_text;

public:
	LabelWidget(unsigned x, unsigned y, unsigned width, unsigned height);
	~LabelWidget(void);

	void clear(void);
	void setText(const char *text, unsigned font, unsigned color,
		unsigned outline = OUTLINE_NONE, unsigned align = ALIGN_LEFT);
	const char *text(void) const;

	void redraw(int x, int y, unsigned curtick);
};

class WidgetContainer : public Recyclable {
private:
	Widget **_widgets;
	size_t _widgetCount, _widgetMax;

	// Do NOT implement
	WidgetContainer(const WidgetContainer &other);
	const WidgetContainer &operator=(const WidgetContainer &other);

protected:
	Widget *_currentWidget;

	Widget *createWidget(unsigned x, unsigned y, unsigned width,
		unsigned height);

	// Add new widget to the container. If the widget cannot be added,
	// it'll be automatically deleted and excetpion will be thrown.
	void addWidget(Widget *w);

	Widget *findWidget(int x, int y);
	void redrawWidgets(int x, int y, unsigned curtick);
	void clearWidgets(void);

public:
	WidgetContainer(void);
	~WidgetContainer(void);

	virtual void redraw(unsigned curtick) = 0;

	virtual void handleMouseMove(int x, int y, unsigned buttons);
	virtual void handleMouseDown(int x, int y, unsigned button);
	virtual void handleMouseUp(int x, int y, unsigned button);
};

class GuiView;

class GuiWindow : public WidgetContainer {
protected:
	GuiView *_parent;
	int _x, _y, _grabx, _graby;
	unsigned _width, _height, _flags;

public:
	explicit GuiWindow(GuiView *parent, unsigned flags = WINDOW_MOVABLE);
	~GuiWindow(void);

	void discard(void);

	virtual int isInside(int x, int y) const;
	int isModal(void) const;
	int isGrabbed(void) const;

	virtual void close(int x = 0, int y = 0, int arg = 0);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseDown(int x, int y, unsigned button);
	void handleMouseUp(int x, int y, unsigned button);
};

class GuiView : public WidgetContainer {
private:
	BilistNode<GuiWindow> _firstWindow, _lastWindow;

protected:
	void addWindow(GuiWindow *window);

	// Removes window from view but does not delete or discard it
	void removeWindow(GuiWindow *window);

	// Moves window to the front, node is discarded in the process
	void focusWindow(BilistNode<GuiWindow> *node);
	BilistNode<GuiWindow> *findWindowAt(int x, int y, int ignore_modal = 0);
	BilistNode<GuiWindow> *findModalWindow(void);
	void redrawWindows(unsigned curtick);

public:
	GuiView(void);
	~GuiView(void);

	// methods called on view transitions (may be called multiple times
	// on the same instance)
	virtual void open(void);
	virtual void close(void);

	// Discard this instance from view stack and switch to the next view
	// (if any). It is safe to access instance variable after calling
	// this method. The instance will be garbage collected after control
	// returns to the main loop.
	void exitView(void);

	void handleMouseMove(int x, int y, unsigned buttons);
	void handleMouseDown(int x, int y, unsigned button);
	void handleMouseUp(int x, int y, unsigned button);

	friend class GuiWindow;
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
	GuiView **_stack;
	size_t _top, _size;

	// Do NOT implement
	ViewStack(const ViewStack &other);
	const ViewStack &operator=(const ViewStack &other);

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
