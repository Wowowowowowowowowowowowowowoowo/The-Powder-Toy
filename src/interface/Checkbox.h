#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <functional>
#include <string>
#include "common/Point.h"
#include "graphics/ARGBColour.h"
#include "Component.h"

class VideoBuffer;
class Checkbox;
class ToolTip;

class Checkbox : public Component
{
	bool checked;
	std::string text;
	bool useCheckIcon = false;
	ToolTip *tooltip;
	std::function<void(bool)> callback = nullptr;

	bool textInside;

public:
	Checkbox(Point position, Point size, std::string text_);
	virtual ~Checkbox();

	bool IsChecked() { return checked; }

	void SetChecked(bool checked_) { checked = checked_; }
	void SetText(std::string text_);
	void UseCheckIcon(bool useIcon) { useCheckIcon = useIcon; }
	void SetTooltip(ToolTip *newTip);
	void SetTooltipText(std::string newTooltip);
	void SetCallback(std::function<void(bool)> callback) { this->callback = callback; }

	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnTick(uint32_t ticks);

	static const int AUTOSIZE = -1;
	static const int TEXTINSIDE = -2;
};

#endif
