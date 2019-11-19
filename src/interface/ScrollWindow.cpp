#include "ScrollWindow.h"
#include "common/tpt-minmax.h"
#include "graphics/VideoBuffer.h"

namespace ui
{
ScrollWindow::ScrollWindow(Point position, Point size):
	Window(position, size),
	scrollable(false),
	scrollSize(size.Y),
	scrolled(0),
	lastMouseX(0),
	lastMouseY(0)
{
}

void ScrollWindow::DoMouseWheel(int x, int y, int d)
{
	if (scrollable)
	{
		lastMouseX = x;
		lastMouseY = y;
		if (d > 0)
			SetScrollPosition(std::max(scrolled-d*4, 0));
		else if (d < 0)
			SetScrollPosition(std::min(scrolled-d*4, scrollSize));
	}

	/*for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		(*iter)->OnMouseWheel(x, y, d);
	}*/

	OnMouseWheel(x, y, d);
}

void ScrollWindow::OnDraw(gfx::VideoBuffer *buf)
{
	if (scrollable)
	{
		float scrollHeight = static_cast<float>(size.Y) * (static_cast<float>(size.Y) / static_cast<float>(scrollSize + size.Y));
		float scrollPos = 0;
		if (scrolled > 0)
			scrollPos = static_cast<float>(size.Y - scrollHeight) * (static_cast<float>(scrolled) / static_cast<float>(scrollSize));

		buf->FillRect(size.X - 8, 0, 8, size.Y, 125, 125, 125, 100);
		buf->FillRect(size.X - 8, static_cast<int>(scrollPos), 8, static_cast<int>(scrollHeight) + 1, 255, 255, 255, 255);
	}
}

void ScrollWindow::OnMouseDown(int x, int y, unsigned char button)
{
	if (isMouseInsideScrollbar)
	{
		scrollbarHeld = true;
		initialClickPos = y;
		initialOffset = scrolled;
	}
}

void ScrollWindow::OnMouseUp(int x, int y, unsigned char button)
{
	scrollbarHeld = false;
}

void ScrollWindow::OnMouseMove(int x, int y, Point difference)
{
	if (scrollable)
	{
		float scrollHeight = static_cast<float>(size.Y) * (static_cast<float>(size.Y) / static_cast<float>(scrollSize + size.Y));
		float scrollPos = 0;
		if (scrolled > 0)
			scrollPos = static_cast<float>(size.Y - scrollHeight) * (static_cast<float>(scrolled) / static_cast<float>(scrollSize));

		isMouseInsideScrollbar = x >= size.X - 8 && x < size.X && y >= scrollPos && y < scrollPos + scrollHeight;

		if (scrollbarHeld)
		{
			int newScrolled;
			if (x < 0)
				newScrolled = initialOffset;
			else
				newScrolled = static_cast<float>(y - initialClickPos) / static_cast<float>(size.Y) * (scrollSize + size.Y) + initialOffset;
			SetScrollPosition(newScrolled);
		}
	}
}

void ScrollWindow::SetScrollable(bool scrollable, int maxScroll)
{
	if (maxScroll < 0)
		return;
	this->scrollable = scrollable;
	scrollSize = maxScroll;
	if (!scrollable)
		SetScrollPosition(0);
	else if (scrolled > scrollSize)
		SetScrollPosition(scrollSize);
}

void ScrollWindow::SetScrollPosition(int pos)
{
	bool alreadyInside = false;
	int oldScrolled = scrolled;

	if (pos < 0)
		pos = 0;
	if (pos > scrollSize)
		pos = scrollSize;

	scrolled = pos;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		temp->SetPosition(Point(temp->GetPosition().X, temp->GetPosition().Y-(scrolled-oldScrolled)));

		int posX = lastMouseX-this->position.X-temp->GetPosition().X, posY = lastMouseY-this->position.Y-temp->GetPosition().Y;
		// update isMouseInside for this component
		if (!alreadyInside && posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y)
		{
			if (!InsideSubwindow(lastMouseX, lastMouseY))
				temp->SetMouseInside(true);
			alreadyInside = true;
		}
		else
			temp->SetMouseInside(false);
	}
}
}
