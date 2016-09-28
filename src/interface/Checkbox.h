#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <string>
#include "common/Point.h"
#include "graphics/ARGBColour.h"
#include "Component.h"

class VideoBuffer;
class Checkbox;
class ToolTip;
class CheckboxAction
{
public:
	virtual ~CheckboxAction() { }
	virtual void CheckboxActionCallback(Checkbox *checkbox, unsigned char b) = 0;
};

class Checkbox : public Component
{
	bool checked;
	std::string text;
	ToolTip *tooltip;
	CheckboxAction *callback;

	bool textInside;

public:
	Checkbox(Point position, Point size, std::string text_);
	virtual ~Checkbox();

	bool IsChecked() { return checked; }

	void SetChecked(bool checked_) { checked = checked_; }
	void SetText(std::string text_);
	void SetTooltip(ToolTip *newTip);
	void SetTooltipText(std::string newTooltip);
	void SetCallback(CheckboxAction *callback_);

	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick(uint32_t ticks);

	static const int AUTOSIZE = -1;
	static const int TEXTINSIDE = -2;
};

#endif
