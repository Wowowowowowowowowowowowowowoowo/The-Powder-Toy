#include "graphics.h"
#include "Engine.h"
#include "EventLoopSDL.h"
#include "Window.h"
#include "common/Point.h"

/**
 * Engine class - manages shown windows and communicates with backend i/o libraries (sdl only right now)
 */

Engine::Engine():
	windows(std::stack<Window_*>()),
	top(NULL),
	nextTop(NULL),
	scale(1)
{

}

Engine::~Engine()
{
	while (!windows.empty())
	{
		delete windows.top();
		windows.pop();
	}
}

void Engine::ProcessWindowUpdates()
{
	// close / open any windows that need to be shown
	CloseWindowDelayed();
	if (nextTop)
		ShowWindowDelayed();
}

// can only show one new window per frame
void Engine::ShowWindow(Window_ *window)
{
	if (!window || nextTop)
		return;
	nextTop = window;
	// show window now if this is the first
	if (!top)
		ShowWindowDelayed();
}

void Engine::ShowWindowDelayed()
{
	if (top)
		top->DoDefocus();
	fillrect(vid_buf, -1, -1, XRES+BARSIZE+1, YRES+MENUSIZE+1, 0, 0, 0, 100);
	windows.push(nextTop);
	top = nextTop;
	nextTop = NULL;
	top->DoFocus();

	// update mouse position on any new windows
	int mx, my;
	CalculateMousePosition(&mx, &my);
	top->DoMouseMove(mx, my, 0, 0);
}

void Engine::CloseWindow(Window_ *window)
{
	if (window == windows.top())
	{
		window->toDelete = true;
	}
}

void Engine::CloseTop()
{
	Window_ *temp = windows.top();
	temp->toDelete = true;
}

void Engine::CloseWindowDelayed()
{
	while (top && (top->toDelete || isShutdown))
	{
		delete top;
		windows.pop();
		if (windows.size())
		{
			top = windows.top();
			top->FocusComponent(NULL);
			// update mouse position on any new windows
			int mx, my;
			CalculateMousePosition(&mx, &my);
			top->DoMouseMove(mx, my, 0, 0);
		}
		else
			top = NULL;

	}
}

unsigned int Engine::GetScale()
{
	return scale;
}

void Engine::SetScale(unsigned int scale)
{
	if (this->scale != scale) {
		this->scale = scale;
		SDLSetScreen(false, kiosk_enable, false);
	}
}

int Engine::GetFullscreen()
{
	return kiosk_enable;
}

void Engine::SetFullscreen(bool fullscreen)
{
	if (fullscreen != kiosk_enable)
		SDLSetScreen(false, fullscreen, false);
}

void Engine::ClipboardPush(std::string text)
{
	PushToClipboard(text);
}

std::string Engine::ClipboardPull()
{
	return PullFromClipboard();
}
