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

	void ShowWindow(ui::Window *window);
	void CloseWindow(ui::Window *window);
	void CloseTop();
	ui::Window * GetTop() { return top; }
	void RestorePreviousBuffer();
	void ProcessWindowUpdates();

	unsigned int GetScale();
	void SetScale(unsigned int scale);

	bool IsResizable();
	void SetResizable(bool resizable, bool recreateWindow);

	int GetPixelFilteringMode();
	void SetPixelFilteringMode(int pixelFilteringMode, bool recreateWindow);

	bool IsFullscreen();
	void SetFullscreen(bool fullscreen);

	bool IsAltFullscreen();
	void SetAltFullscreen(bool altFullscreen);

	bool IsForceIntegerScaling();
	void SetForceIntegerScaling(bool forceIntegerScaling);

	bool IsFastQuit() { return fastQuit; }
	void SetFastQuit(bool fastQuit) { this->fastQuit = fastQuit; }

	void ClipboardPush(std::string text);
	std::string ClipboardPull();
	int GetModifiers();

private:
	void ShowWindowDelayed();
	void CloseWindowDelayed();

	std::stack<ui::Window*> windows;
	std::stack<pixel*> buffers;
	ui::Window *top, *nextTop;
	bool isShutdown = false;

	unsigned int scale = 1;
	bool resizable = false;
	int pixelFilteringMode = 0;
	bool fullscreen = false;
	bool altFullscreen = false;
	bool forceIntegerScaling = true;

	bool fastQuit = false;
};

#endif
