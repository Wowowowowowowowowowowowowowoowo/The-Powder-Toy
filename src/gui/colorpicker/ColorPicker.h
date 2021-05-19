#ifndef COLORPICKER_H
#define COLORPICKER_H
#include <functional>
#include "interface/Window.h"

class Label;
class Button;
class Textbox;
class ColorPicker : public ui::Window
{
	std::function<void(ARGBColour)> callback = nullptr;

	int currentHue = 0;
	int currentSaturation = 0;
	int currentValue = 0;
	int currentAlpha = 0;

	bool mouseDown = false;
	bool valueMouseDown = false;

	Textbox *rValue;
	Textbox *gValue;
	Textbox *bValue;
	Textbox *aValue;
	Label *hexValue;

	void UpdateTextboxes(int r, int g, int b, int a);
public:
	ColorPicker(ARGBColour initialColor, std::function<void(int)> callback);

	void OnDraw(gfx::VideoBuffer *buf) override;
	void OnMouseMove(int x, int y, Point difference) override;
	void OnMouseDown(int x, int y, unsigned char button) override;
	void OnMouseUp(int x, int y, unsigned char button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
};

#endif
