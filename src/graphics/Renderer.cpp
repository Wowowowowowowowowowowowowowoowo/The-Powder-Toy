#include "powdergraphics.h"
#include "Renderer.h"

Renderer::Renderer():
	renderModes(std::set<unsigned int>()),
	displayModes(std::set<unsigned int>()),
	colorMode(0)
{
	AddRenderMode(RENDER_FIRE);
	AddRenderMode(RENDER_SPRK);
	AddRenderMode(RENDER_EFFE);
	AddRenderMode(RENDER_BASC);
}

bool Renderer::HasRenderMode(unsigned int renderMode)
{
	return renderModes.find(renderMode) != renderModes.end();
}

void Renderer::AddRenderMode(unsigned int renderMode)
{
	renderModes.insert(renderMode);
}

void Renderer::RemoveRenderMode(unsigned int renderMode)
{
	renderModes.erase(renderMode);
}

void Renderer::ClearRenderModes()
{
	renderModes.clear();
}

std::set<unsigned int> Renderer::GetRenderModes()
{
	return renderModes;
}

unsigned int Renderer::GetRenderModesRaw()
{
	unsigned int render_mode = 0;
	for (std::set<unsigned int>::iterator it = renderModes.begin(), end = renderModes.end(); it != end; it++)
		render_mode |= (*it);
	return render_mode;
}

void Renderer::SetRenderModes(std::set<unsigned int> newRenderModes)
{
	renderModes = std::set<unsigned int>(newRenderModes);
}


bool Renderer::HasDisplayMode(unsigned int displayMode)
{
	return displayModes.find(displayMode) != displayModes.end();
}

void Renderer::AddDisplayMode(unsigned int displayMode)
{
	displayModes.insert(displayMode);
}

void Renderer::RemoveDisplayMode(unsigned int displayMode)
{
	displayModes.erase(displayMode);
}

void Renderer::ClearDisplayModes()
{
	displayModes.clear();
}

std::set<unsigned int> Renderer::GetDisplayModes()
{
	return displayModes;
}

unsigned int Renderer::GetDisplayModesRaw()
{
	unsigned int display_mode = 0;
	for (std::set<unsigned int>::iterator it = displayModes.begin(), end = displayModes.end(); it != end; it++)
		display_mode |= (*it);
	return display_mode;
}

void Renderer::SetDisplayModes(std::set<unsigned int> newDisplayModes)
{
	displayModes = std::set<unsigned int>(newDisplayModes);
}

void Renderer::SetColorMode(unsigned int color_mode)
{
	colorMode = color_mode;
}

void Renderer::XORColorMode(unsigned int color_mode)
{
	colorMode ^= color_mode;
}

unsigned int Renderer::GetColorMode()
{
	return colorMode;
}
