#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include "common/Point.h"
#include "graphics/ARGBColour.h"
#include "Component.h"

class VideoBuffer;
class Button;
class ToolTip;
class ButtonAction
{
public:
	virtual ~ButtonAction() { }
	virtual void ButtionActionCallback(Button *button, unsigned char b) = 0;
};

class Button : public Component
{
public:
	enum TextAlign { LEFT, CENTER };
	enum State { NORMAL, HIGHLIGHTED, INVERTED, HOLD };
	static const uint32_t heldThreshold = 1000;

private:
	std::string text;
	ARGBColour textColor;
	ToolTip *tooltip;
	ButtonAction *callback;
	TextAlign alignment;
	State state;
	bool isCloseButton;

	// "hold" button does different actions when you hold it for one second
	uint32_t timeHeldDown;
	bool didVibrate;

public:
	Button(Point position, Point size, std::string text_);
	virtual ~Button();

	void SetText(std::string text_);
	void SetColor(ARGBColour newColor);
	void SetTextColor(ARGBColour newColor);
	void SetTooltip(ToolTip *newTip);
	void SetTooltipText(std::string newTooltip);
	void SetCallback(ButtonAction *callback_);
	void SetAlign(TextAlign align) { alignment = align; }
	void SetState(State state_) { state = state_; }
	void SetCloseButton(bool isCloseButton_) { isCloseButton = isCloseButton_; }

	bool IsHeld() { return timeHeldDown > heldThreshold; }

	virtual void OnMouseDown(int x, int y, unsigned char button);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnMouseMoved(int x, int y, Point difference);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick(uint32_t ticks);

	static const int AUTOSIZE = -1;
};

#endif
