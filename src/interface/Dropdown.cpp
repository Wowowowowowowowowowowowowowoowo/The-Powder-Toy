#include "Dropdown.h"
#include "common/tpt-minmax.h"
#include "graphics/VideoBuffer.h"
#include "interface/Window.h"

Dropdown::Dropdown(Point position, Point size, std::vector<std::string> options):
	Component(position, size),
	options(options)
{
	if (size.X == AUTOSIZE)
	{
		int maxWidth = 25;
		for (auto option : options)
			maxWidth = tpt::max(maxWidth, VideoBuffer::TextSize(option).X);
		this->size.X = maxWidth + 5;
	}
	if (size.Y == AUTOSIZE)
	{
#ifndef TOUCHUI
		this->size.Y = 16;
#else
		this->size.Y = 24;
#endif
	}
}

void Dropdown::SetSelectedOption(unsigned int selectedOption)
{
	if (selectedOption < options.size())
		this->selectedOption = selectedOption;
}

void Dropdown::OnMouseUp(int x, int y, unsigned char button)
{
	if (IsClicked())
	{
		Point windowPos = position;
		Point windowSize = size;
		windowPos.X -= 2;
		windowSize.X += 4;

		int numOptions = options.size();
		int optionHeight = windowSize.Y;
		windowPos.Y = windowPos.Y - ((numOptions - 1) * optionHeight) / 2 + 1;
		windowSize.Y = numOptions * (optionHeight - 1) + 1;

		DropdownOptions *dropdownOptions = new DropdownOptions(windowPos, windowSize, this);
		this->GetParent()->AddSubwindow(dropdownOptions);
		isSelectingOption = true;
	}
}

void Dropdown::OnDraw(VideoBuffer* vid)
{
	ARGBColour col = color;
	if (!enabled)
		col = COLARGB(COLA(col), (int)(COLR(col) * .55f), (int)(COLG(col) * .55f), (int)(COLB(col) * .55f));
	vid->DrawRect(position.X, position.Y, size.X, size.Y, COLR(col), COLG(col), COLB(col), COLA(col));
	if (selectedOption < options.size())
		vid->DrawText(position.X + 3, position.Y + (size.Y / 2) - 4, options[selectedOption], COLR(col), COLG(col), COLB(col), COLA(col));
}


DropdownOptions::DropdownOptions(Point position, Point size, Dropdown * dropdown):
	Window_(position, size),
	dropdown(dropdown)
{
	optionHeight = dropdown->size.Y - 1;
}

DropdownOptions::~DropdownOptions()
{
	dropdown->isSelectingOption = false;
}

void DropdownOptions::OnMouseMove(int x, int y, Point difference)
{
#ifndef TOUCHUI
	if (Point(x, y).IsInside(position, position + size))
		hoveredOption = tpt::min((size_t)((y - position.Y) / optionHeight), dropdown->options.size() - 1);
	else
		hoveredOption = -1;
#endif
}

void DropdownOptions::OnMouseUp(int x, int y, unsigned char button)
{
	if (Point(x, y).IsInside(position, position + size))
	{
		dropdown->selectedOption = (y - position.Y) / optionHeight;
		if (dropdown->callback)
			dropdown->callback(dropdown->selectedOption);
	}
	this->toDelete  = true;
}

void DropdownOptions::OnDraw(VideoBuffer* vid)
{
	int optionHeight = dropdown->size.Y;
	for (size_t i = 0; i < dropdown->options.size(); i++)
	{
		int yPos = (i * (optionHeight-1));
		if (i == dropdown->selectedOption)
			vid->FillRect(0, yPos, size.X, optionHeight, 75, 75, 75, 255);
		else if (i == hoveredOption)
			vid->FillRect(0, yPos, size.X, optionHeight, 50, 50, 50, 255);
		else
			vid->FillRect(0, yPos, size.X, optionHeight, 0, 0, 0, 255);
		vid->DrawRect(0, yPos, size.X, optionHeight, 255, 255, 255, 255);
		vid->DrawText(3, yPos + 4, dropdown->options[i], 255, 255, 255, 255);

	}
}
