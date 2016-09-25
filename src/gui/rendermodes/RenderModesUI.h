#ifndef RENDERMODES_H
#define RENDERMODES_H
#include <string>
#include <vector>
#include "interface/Window.h"

class Checkbox;
class RenderModesUI : public Window_
{
	int line1Pos;
	int line2Pos;
	unsigned int last_render_mode;
	unsigned int last_display_mode;
	unsigned int last_color_mode;


	std::vector<std::pair<Checkbox*, unsigned int> > renderCheckboxes;
	std::vector<std::pair<Checkbox*, unsigned int> > displayCheckboxes;
	std::vector<std::pair<Checkbox*, unsigned int> > colorCheckboxes;

	void InitializeRenderCheckbox(Checkbox *checkbox, unsigned int mode);
	void InitializeDisplayCheckbox(Checkbox *checkbox, unsigned int mode);
	void InitializeColorCheckbox(Checkbox *checkbox, unsigned int mode);
	void SetCheckboxToolTip(Checkbox *checkbox, std::string tooltip);
	void InitializeCheckboxes();
	void InitializeButtons();

public:
	RenderModesUI();

	virtual void OnTick(uint32_t ticks);
	virtual void OnDraw(VideoBuffer *buf);
	virtual void OnKeyPress(int key, unsigned short character, unsigned short modifiers);
};

#endif
