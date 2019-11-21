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

void ScrollWindow::OnDrawBeforeComponents(gfx::VideoBuffer *buf)
{
	if (onDraw != nullptr)
		onDraw(buf);
}

void ScrollWindow::OnDraw(gfx::VideoBuffer *buf)
{
	if (scrollable)
	{
		float scrollHeight = static_cast<float>(size.Y) * (static_cast<float>(size.Y) / static_cast<float>(scrollSize + size.Y));
		float scrollPos = 0;
		if (scrolled > 0)
			scrollPos = static_cast<float>(size.Y - scrollHeight) * (static_cast<float>(scrolled) / static_cast<float>(scrollSize));

		buf->FillRect(size.X - scrollbarWidth, 0, scrollbarWidth, size.Y, 125, 125, 125, 100);
		buf->FillRect(size.X - scrollbarWidth, static_cast<int>(scrollPos), scrollbarWidth, static_cast<int>(scrollHeight) + 1, 255, 255, 255, 255);
	}
}

void ScrollWindow::DoMouseDown(int x, int y, unsigned char button)
{
	if (isMouseInsideScrollbar)
	{
		scrollbarHeld = true;
		initialClickX = x;
		initialClickY = y;
		initialOffset = scrolled;
	}

#ifndef TOUCHUI
	Window::DoMouseDown(x, y, button);
#else
	if (!isMouseInsideScrollbar)
		Window::DoMouseDown(x, y, button);
#endif
}

#ifdef TOUCHUI
void ScrollWindow::DoMouseUp(int x, int y, unsigned char button)
{
	if (!hasScrolled)
	{
		Window::DoMouseDown(initialClickX, initialClickY, button);
		Window::DoMouseMove(x, y, x - initialClickX, y - initialClickY);
	}
	hasScrolled = false;
	Window::DoMouseUp(x, y, button);
}
#endif

void ScrollWindow::OnMouseUp(int x, int y, unsigned char button)
{
	scrollbarHeld = false;
}

void ScrollWindow::OnMouseMove(int x, int y, Point difference)
{
	if (scrollable)
	{
#ifndef TOUCHUI
		float scrollHeight = static_cast<float>(size.Y) * (static_cast<float>(size.Y) / static_cast<float>(scrollSize + size.Y));
		float scrollPos = 0;
		if (scrolled > 0)
			scrollPos = static_cast<float>(size.Y - scrollHeight) * (static_cast<float>(scrolled) / static_cast<float>(scrollSize));

		isMouseInsideScrollbar = x >= size.X - scrollbarWidth && x < size.X && y >= scrollPos && y < scrollPos + scrollHeight;
#else
		isMouseInsideScrollbar = true;
#endif

		if (scrollbarHeld && Subwindows.size() == 0)
		{
			int newScrolled;
			if (x < initialClickX - 100)
				newScrolled = initialOffset;
			else
			{
#ifndef TOUCHUI
				newScrolled = static_cast<float>(y - initialClickY) / static_cast<float>(size.Y) * (scrollSize + size.Y) + initialOffset;
#else
				// 10 pixels of leeway before it starts scrolling
				int diff = initialClickY - y;
				if (!hasScrolled)
				{
					if (std::abs(diff) < 10)
						diff = 0;
					else if (diff < 0)
						diff += 10;
					else if (diff > 0)
						diff -= 10;
					// Block events from being sent to components once we enter scrolling mode
					if (diff != 0)
					{
						initialClickX = x;
						initialClickY = y;
						hasScrolled = true;
						diff = 0;
					}
				}
				newScrolled = static_cast<float>(diff) / static_cast<float>(size.Y) * (scrollSize + size.Y) + initialOffset;
#endif
			}
			lastMouseX = x;
			lastMouseY = y;
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
