#include "Checkbox.h"
#include "Window.h"
#include "common/tpt-minmax.h"
#include "game/ToolTip.h"
#include "graphics/VideoBuffer.h"

Checkbox::Checkbox(Point position, Point size_, std::string text_):
	Component(position, size_),
	checked(false),
	tooltip(NULL),
	callback(NULL)
{
	if (size.X == AUTOSIZE || size.Y == AUTOSIZE)
	{
		Point checkboxSize = VideoBuffer::TextSize(text_);
		if (size.Y == AUTOSIZE)
			size.Y = 16;
		if (size.X == AUTOSIZE)
			size.X = checkboxSize.X + size.Y + 3;
	}
	SetText(text_);
}

Checkbox::~Checkbox()
{
	delete callback;
	delete tooltip;
}

void Checkbox::SetText(std::string text_)
{
	text = text_;
	// ensure text isn't too big for button, maybe not too efficient
	if (VideoBuffer::TextSize(text).X+size.Y+2 >= size.X)
	{
		text += "...";
		while (text.length() > 3 && VideoBuffer::TextSize(text).X+size.Y+2 >= size.X)
		{
			text.erase(text.length()-4, 1);
		}
	}
}

void Checkbox::SetTooltip(ToolTip *newTip)
{
	delete tooltip;
	tooltip = newTip;
}

void Checkbox::SetTooltipText(std::string newTooltip)
{
	if (tooltip)
		tooltip->SetTip(newTooltip);
}

void Checkbox::SetCallback(CheckboxAction *callback_)
{
	delete callback;
	callback = callback_;
}

void Checkbox::OnMouseUp(int x, int y, unsigned char button)
{
	if (IsClicked() && isMouseInside && enabled)
	{
		checked = !checked;
		if (callback)
		{
			callback->CheckboxActionCallback(this, button);
		}
	}
}

void Checkbox::OnDraw(VideoBuffer* vid)
{
	// clear area
	vid->FillRect(position.X, position.Y, size.X, size.Y, 0, 0, 0, 255);

	// border
	if (enabled)
		vid->DrawRect(position.X, position.Y, size.Y, size.Y, COLR(color), COLG(color), COLB(color), 255);
	else
		vid->DrawRect(position.X, position.Y, size.Y, size.Y, (int)(COLR(color)*.55f), (int)(COLG(color)*.55f), (int)(COLB(color)*.55f), 255);

	ARGBColour realTextColor = 0;
	ARGBColour backgroundColor = 0;
	if (!enabled)
	{
		realTextColor = COLRGB((int)(COLR(color)*.55f), (int)(COLG(color)*.55f), (int)(COLB(color)*.55f));
	}
#ifdef TOUCHUI
	// Mouse not inside button, Mouse not down, or over button but click did not start on button
	else if (!isMouseInside || !IsMouseDown() || (IsMouseDown() && !IsClicked()))
#else
	// Mouse not inside button, or over button but click did not start on button
	else if (!isMouseInside || (IsMouseDown() && !IsClicked()))
#endif
	{
		if (checked)
		{
			backgroundColor = color;
			realTextColor = color;
		}
		else
		{
			realTextColor = COLARGB(150, COLR(color), COLG(color), COLB(color));
		}
	}
	else
	{
		// button clicked and held down
		if (IsClicked())
		{
			if (checked)
			{
				realTextColor = COLARGB(150, COLR(color), COLG(color), COLB(color));
			}
			else
			{
				backgroundColor = color;
				realTextColor = color;
			}
		}
		// Mouse over button, not held down
		else
		{
			if (checked)
			{
				backgroundColor = COLARGB(200, COLR(color), COLG(color), COLB(color));
				realTextColor = COLARGB(200, COLR(color), COLG(color), COLB(color));
			}
			else
			{
				backgroundColor = COLARGB(125, COLR(color), COLG(color), COLB(color));
				realTextColor = COLARGB(225, COLR(color), COLG(color), COLB(color));
			}
		}
	}
	// background color (if required)
	if (backgroundColor)
	{
		vid->FillRect(position.X+3, position.Y+3, size.Y-6, size.Y-6, COLR(backgroundColor), COLG(backgroundColor), COLB(backgroundColor), COLA(backgroundColor));
	}

	Point textSize = VideoBuffer::TextSize(text);
	vid->DrawText(position.X+size.Y+2, position.Y+(size.Y-textSize.Y+1)/2+1, text, COLR(realTextColor), COLG(realTextColor), COLB(realTextColor), COLA(realTextColor));
}

void Checkbox::OnTick(uint32_t ticks)
{
#ifdef TOUCHUI
	if (isMouseInside && enabled && IsMouseDown() && tooltip)
#else
	if (isMouseInside && enabled && tooltip)
#endif
		tooltip->AddToScreen();
}
