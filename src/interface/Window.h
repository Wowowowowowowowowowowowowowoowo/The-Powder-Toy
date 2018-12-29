#ifndef WINDOW_H
#define WINDOW_H

#include <vector>
#include "common/tpt-stdint.h"
#include "common/Point.h"
#include "common/SDL_keysym.h"
#include "graphics/Pixel.h"
#include "Component.h"

class VideoBuffer;
class Window_
{
public:
	Window_(Point position, Point size);
	virtual ~Window_();
	void Resize(Point position, Point size);

	void SetParent(Window_ *parentWindow) { parent = parentWindow; }
	Window_* GetParent() { return parent; }
	void AddSubwindow(Window_ *other);
	void RemoveSubwindow(Window_ *other);
	void AddComponent(Component *other);
	void RemoveComponent(Component *other);
	void FocusComponent(Component *toFocus);
	void UpdateComponents();
	bool IsFocused(const Component *other) const { return other == focused; }
	bool IsClicked(const Component *other) const { return other == clicked; }

	void DoExit(); // calls OnExit, doesn't actually exit though
	void DoFocus();
	void DoDefocus();
	void DoTick(uint32_t ticks);
	void DoDraw(pixel *copyBuf, Point copySize, Point copyPos);

	void DoMouseMove(int x, int y, int dx, int dy);
	void DoMouseDown(int x, int y, unsigned char button);
	void DoMouseUp(int x, int y, unsigned char button);
	void DoMouseWheel(int x, int y, int d);
	void DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void DoKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void DoTextInput(const char *text);
	void DoJoystickMotion(uint8_t joysticknum, uint8_t axis, int16_t value);

	Point GetPosition() { return position; }
	Point GetSize() { return size; }
	VideoBuffer* GetVid() { return videoBuffer; }
	bool IsMouseDown() { return isMouseDown; }
	bool CanQuit() { return !ignoreQuits; }
	void HasBorder(bool border) { hasBorder = border; }

	bool toDelete;

	static const int CENTERED = -1;

protected:
	Point position;
	Point size;
	std::vector<Component*> Components;
	std::vector<Window_*> Subwindows;
	bool isMouseDown; // need to keep track of this for some things like buttons
	bool ignoreQuits;
	bool hasBorder;

	virtual void OnExit() { }
	virtual void OnTick(uint32_t ticks) { }
	virtual void OnDraw(VideoBuffer *buf) { }
	virtual void OnDrawAfterSubwindows(VideoBuffer *buf) { }
	virtual void OnMouseMove(int x, int y, Point difference) { }
	virtual void OnMouseDown(int x, int y, unsigned char button) { }
	virtual void OnMouseUp(int x, int y, unsigned char button) { }
	virtual void OnMouseWheel(int x, int y, int d) { }
	virtual void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) { }
	virtual void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) { }
	virtual void OnTextInput(const char *text) { }
	virtual void OnJoystickMotion(uint8_t joysticknum, uint8_t axis, int16_t value) { }

	virtual void OnFocus() { }
	virtual void OnDefocus() { }

	// these functions are called before components are updated, and can be used to cancel events before sending them to the components
	virtual bool BeforeMouseMove(int x, int y, Point difference) { return true; }
	virtual bool BeforeMouseDown(int x, int y, unsigned char button) { return true; }
	virtual bool BeforeMouseUp(int x, int y, unsigned char button) { return true; }
	virtual bool BeforeMouseWheel(int x, int y, int d) { return true; }
	virtual bool BeforeKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) { return true; }
	virtual bool BeforeKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) { return true; }
	virtual bool BeforeTextInput(const char *text) { return true; }

	void VideoBufferHack();
	bool InsideSubwindow(int x, int y);

private:
	Window_ *parent;
	bool mouseDownOutside;
	VideoBuffer *videoBuffer;
	Component *focused;
	Component *clicked;
};

#endif
