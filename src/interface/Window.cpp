#include "Window.h"
#include "Engine.h"
#include "Label.h"
#include "Textbox.h"
#include "graphics.h"
#include "graphics/VideoBuffer.h"

Window_::Window_(Point position_, Point size_):
	toDelete(false),
	position(position_),
	size(size_),
	Components(std::vector<Component*>()),
	Subwindows(std::vector<Window_*>()),
	isMouseDown(false),
	ignoreQuits(false),
	hasBorder(true),
	parent(NULL),
	mouseDownOutside(false),
	focused(NULL),
	clicked(NULL)
{
	if (position.X == CENTERED)
		position.X = (XRES+BARSIZE-size.X)/2;
	if (position.Y == CENTERED)
		position.Y = (YRES+MENUSIZE-size.Y)/2;
	videoBuffer = new VideoBuffer(size.X, size.Y);
}

Window_::~Window_()
{
	delete videoBuffer;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		delete *iter;
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
		delete *iter;
	Components.clear();
	Subwindows.clear();
}

void Window_::Resize(Point position_, Point size_)
{
	delete videoBuffer;
	position = position_;
	size = size_;
	if (position.X == CENTERED)
		position.X = (XRES+BARSIZE-size.X)/2;
	if (position.Y == CENTERED)
		position.Y = (YRES+MENUSIZE-size.Y)/2;
	videoBuffer = new VideoBuffer(size.X, size.Y);
}

void Window_::AddComponent(Component *other)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		// this component is already part of the window
		if ((*iter) == other)
		{
			return;
		}
	}
	Components.push_back(other);
	other->SetParent(this);
	other->SetVisible(false);
	other->toAdd = true;
}

void Window_::AddSubwindow(Window_ *other)
{
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		// this component is already part of the window
		if ((*iter) == other)
		{
			return;
		}
	}
	Subwindows.push_back(other);
	other->HasBorder(false);
	other->SetParent(this);
}

void Window_::RemoveComponent(Component *other)
{
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter) == other)
		{
			(*iter)->toDelete = true;
		}
	}
	if (other == focused)
		FocusComponent(NULL);
	if (other == clicked)
		clicked = NULL;
}

void Window_::RemoveSubwindow(Window_ *other)
{
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		if ((*iter) == other)
		{
			(*iter)->toDelete = true;
		}
	}
}

void Window_::FocusComponent(Component *toFocus)
{
	if (focused != toFocus)
	{
		if (focused)
			focused->OnDefocus();
		focused = toFocus;
		if (focused)
			focused->OnFocus();
	}
}

void Window_::UpdateComponents()
{
	for (int i = Components.size()-1; i >= 0; i--)
	{
		if (Components[i]->toDelete)
		{
			Components[i]->SetParent(NULL);
			delete Components[i];
			Components.erase(Components.begin()+i);
		}
		else if (Components[i]->toAdd)
		{
			Components[i]->SetVisible(true);
		}
	}
	for (int i = Subwindows.size()-1; i >= 0; i--)
	{
		Subwindows[i]->UpdateComponents();
		if (Subwindows[i]->toDelete)
		{
			delete Subwindows[i];
			Subwindows.erase(Subwindows.begin()+i);
		}
	}
}

void Window_::DoExit()
{
	OnExit();
}

void Window_::DoFocus()
{
	OnFocus();
}

void Window_::DoDefocus()
{
	OnDefocus();
}

void Window_::DoTick(uint32_t ticks)
{
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoTick(ticks);
	}
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible())
			(*iter)->OnTick(ticks);
	}

	OnTick(ticks);
}

void Window_::DoDraw(pixel *copyBuf, Point copySize, Point copyPos)
{
	// too lazy to create another variable which is temporary anyway
	if (!ignoreQuits)
		videoBuffer->Clear();
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible() && !IsFocused(*iter))
			(*iter)->OnDraw(videoBuffer);
	}
	// draw the focused component on top
	if (focused)
		focused->OnDraw(videoBuffer);
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoDraw(videoBuffer->GetVid(), size, (*iter)->GetPosition());
	}

	OnDraw(videoBuffer);

	if (hasBorder)
		videoBuffer->DrawRect(0, 0, size.X, size.Y, 255, 255, 255, 255);
	if (copyBuf)
		videoBuffer->CopyBufferInto(copyBuf, copySize.X, copySize.Y, copyPos.X, copyPos.Y);
}

void Window_::DoMouseMove(int x, int y, int dx, int dy)
{
	if (!BeforeMouseMove(x, y, Point(dx, dy)))
		return;
	
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoMouseMove(x-position.X, y-position.Y, dx, dy);
	}

	bool alreadyInside = false;
	if (dx || dy)
	{
		for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
		{
			Component *temp = *iter;
			if (temp->IsVisible() && temp->IsEnabled())
			{
				Component *temp = *iter;
				int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
				// update isMouseInside for this component
				if (!alreadyInside && posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y)
				{
					if (!InsideSubwindow(x, y))
						temp->SetMouseInside(true);
					alreadyInside = true;
				}
				else
				{
					temp->SetMouseInside(false);
					if (temp == clicked)
						clicked = NULL;
				}
				temp->OnMouseMoved(posX, posY, Point(dx, dy));
			}
		}
	}

	OnMouseMove(x, y, Point(dx, dy));
}

void Window_::DoMouseDown(int x, int y, unsigned char button)
{
	if (!BeforeMouseDown(x, y, button))
		return;
	
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoMouseDown(x-position.X, y-position.Y, button);
	}

	if (InsideSubwindow(x, y))
		return;
	if (x < position.X || x > position.X+size.X || y < position.Y || y > position.Y+size.Y)
	{
#ifndef TOUCHUI
		mouseDownOutside = true;
#endif
		return;
	}
	isMouseDown = true;

	bool focusedSomething = false;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		if (temp->IsVisible() && temp->IsEnabled())
		{
			int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
			bool inside = posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y;
			if (inside)
			{
				focusedSomething = true;
				FocusComponent(temp);
				clicked = temp;
				temp->OnMouseDown(posX, posY, button);
				break;
			}
		}
	}
	if (!focusedSomething)
		FocusComponent(NULL);

	OnMouseDown(x, y, button);
}

void Window_::DoMouseUp(int x, int y, unsigned char button)
{
	if (!BeforeMouseUp(x, y, button))
		return;
	
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoMouseUp(x-position.X, y-position.Y, button);
	}

#ifndef TOUCHUI
	if (mouseDownOutside)
	{
		mouseDownOutside = false;
		if (hasBorder && (x < position.X || x > position.X+size.X || y < position.Y || y > position.Y+size.Y))
		{
			toDelete = true;
		}
	}
#endif
	isMouseDown = false;
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		Component *temp = *iter;
		if (temp->IsVisible() && temp->IsEnabled())
		{
			int posX = x-this->position.X-temp->GetPosition().X, posY = y-this->position.Y-temp->GetPosition().Y;
			bool inside = posX >= 0 && posX < temp->GetSize().X && posY >= 0 && posY < temp->GetSize().Y;

			if (inside || IsClicked(temp) || IsFocused(temp))
				temp->OnMouseUp(posX, posY, button);
		}
	}
	clicked = NULL;

	OnMouseUp(x, y, button);
}

void Window_::DoMouseWheel(int x, int y, int d)
{
	if (!BeforeMouseWheel(x, y, d))
		return;
	
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoMouseWheel(x-position.X, y-position.Y, d);
	}

	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if ((*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnMouseWheel(x, y, d);
	}

	OnMouseWheel(x, y, d);
}

void Window_::DoKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (!BeforeKeyPress(key, character, modifiers))
		return;

	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoKeyPress(key, character, modifiers);
	}
	
	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter) && (*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnKeyPress(key, character, modifiers);
	}

	OnKeyPress(key, character, modifiers);
}

void Window_::DoKeyRelease(int key, unsigned short character, unsigned short modifiers)
{
	if (!BeforeKeyRelease(key, character, modifiers))
		return;
	
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoKeyRelease(key, character, modifiers);
	}

	for (std::vector<Component*>::iterator iter = Components.begin(), end = Components.end(); iter != end; iter++)
	{
		if (IsFocused(*iter) && (*iter)->IsVisible() && (*iter)->IsEnabled())
			(*iter)->OnKeyRelease(key, character, modifiers);
	}

	OnKeyRelease(key, character, modifiers);
}

void Window_::DoJoystickMotion(uint8_t joysticknum, uint8_t axis, int16_t value)
{
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		(*iter)->DoJoystickMotion(joysticknum, axis, value);
	}

	OnJoystickMotion(joysticknum, axis, value);
}

void Window_::VideoBufferHack()
{
	std::copy(&vid_buf[0], &vid_buf[(XRES+BARSIZE)*(YRES+MENUSIZE)], &videoBuffer->GetVid()[0]);
}

bool Window_::InsideSubwindow(int x, int y)
{
	for (std::vector<Window_*>::iterator iter = Subwindows.begin(), end = Subwindows.end(); iter != end; iter++)
	{
		Window_* wind = (*iter);
		if (Point(x, y).IsInside(wind->GetPosition(), wind->GetPosition()+wind->GetSize()))
		{
			return true;
		}
	}
	return false;
}
