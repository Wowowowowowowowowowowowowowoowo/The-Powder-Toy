#ifndef ENGINE_H
#define ENGINE_H

#include <stack>
#include <string>
#include "Window.h"
#include "common/Singleton.h"
#include "common/tpt-stdint.h"
#include "common/Point.h"

class Engine : public Singleton<Engine>
{
public:
	Engine();
	~Engine();

	bool Running() { return top != nullptr; }
	void Shutdown() { isShutdown = true; }
	bool IsShutdown() { return isShutdown; }

	void ShowWindow(Window_ *window);
	void CloseWindow(Window_ *window);
	void CloseTop();
	Window_ * GetTop() { return top; }
	void ProcessWindowUpdates();

	unsigned int GetScale();
	void SetScale(unsigned int scale);

	int GetFullscreen();
	void SetFullscreen(bool fullscreen);

	void ClipboardPush(std::string text);
	std::string ClipboardPull();

private:
	void ShowWindowDelayed();
	void CloseWindowDelayed();

	std::stack<Window_*> windows;
	Window_ *top, *nextTop;
	bool isShutdown = false;

	unsigned int scale;
};

#endif
