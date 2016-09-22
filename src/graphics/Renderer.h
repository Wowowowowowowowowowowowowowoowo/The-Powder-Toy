#ifndef RENDERER_H
#define RENDERER_H

#include <set>
#include "common/Singleton.h"

// This class is mostly unused at the moment, but is used for controlling render / display modes
class Renderer : public Singleton<Renderer>
{
	std::set<unsigned int> renderModes;
	std::set<unsigned int> displayModes;
	unsigned int colorMode;
public:
	Renderer();

	// render modes
	bool HasRenderMode(unsigned int renderMode);
	void AddRenderMode(unsigned int renderMode);
	void RemoveRenderMode(unsigned int renderMode);
	void ClearRenderModes();

	std::set<unsigned int> GetRenderModes();
	unsigned int GetRenderModesRaw();
	void SetRenderModes(std::set<unsigned int> newRenderModes);

	// display modes
	bool HasDisplayMode(unsigned int displayMode);
	void AddDisplayMode(unsigned int displayMode);
	void RemoveDisplayMode(unsigned int displayMode);
	void ClearDisplayModes();

	std::set<unsigned int> GetDisplayModes();
	unsigned int GetDisplayModesRaw();
	void SetDisplayModes(std::set<unsigned int> newDisplayModes);

	// color modes
	void SetColorMode(unsigned int color_mode);
	void XORColorMode(unsigned int color_mode);
	unsigned int GetColorMode();
};

#endif // RENDERER_H
