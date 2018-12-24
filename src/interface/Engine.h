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

	bool IsResizable();
	void SetResizable(bool resizable);

	bool IsFullscreen();
	void SetFullscreen(bool fullscreen);

	bool IsAltFullscreen();
	void SetAltFullscreen(bool altFullscreen);

	bool IsFastQuit() { return fastQuit; }
	void SetFastQuit(bool fastQuit) { this->fastQuit = fastQuit; }

	void ClipboardPush(std::string text);
	std::string ClipboardPull();
	int GetModifiers();

private:
	void ShowWindowDelayed();
	void CloseWindowDelayed();

	std::stack<Window_*> windows;
	Window_ *top, *nextTop;
	bool isShutdown = false;

	unsigned int scale = 1;
	bool resizable = false;
	bool fullscreen = false;
	bool altFullscreen = false;

	bool fastQuit = false;
};

#endif
