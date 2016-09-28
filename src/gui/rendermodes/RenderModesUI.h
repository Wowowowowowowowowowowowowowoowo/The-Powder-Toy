#ifndef RENDERMODES_H
#define RENDERMODES_H
#include <string>
#include <vector>
#include "interface/Window.h"

class Checkbox;
class Button;
class RenderModesUI : public Window_
{
	int line1Pos;
	int line2Pos;
	unsigned int last_render_mode;
	unsigned int last_display_mode;
	unsigned int last_color_mode;
	bool interfaceSwap; //for TOUCH_UI

	std::vector<std::pair<Checkbox*, unsigned int> > renderCheckboxes;
	std::vector<std::pair<Checkbox*, unsigned int> > displayCheckboxes;
	std::vector<std::pair<Checkbox*, unsigned int> > colorCheckboxes;
	std::vector<Button*> buttons;
	Button *swapButton;

	void InitializeRenderCheckbox(Checkbox *checkbox, unsigned int mode);
	void InitializeDisplayCheckbox(Checkbox *checkbox, unsigned int mode);
	void InitializeColorCheckbox(Checkbox *checkbox, unsigned int mode);
	void SetCheckboxToolTip(Checkbox *checkbox, std::string tooltip);
	Point CheckboxPos(Checkbox *prev1, Checkbox *prev2);
	void InitializeCheckboxes();
	Point ButtonPos(Button *prev1, Button *prev2);
	void InitializeButton(Button *button, int preset);
	void SetButtonToolTip(Button *button, std::string tooltip);
	void InitializeButtons();

public:
	RenderModesUI();

	void SwapInterface();

	virtual void OnTick(uint32_t ticks);
	virtual void OnDraw(VideoBuffer *buf);
	virtual void OnKeyPress(int key, unsigned short character, unsigned short modifiers);
};

#endif
