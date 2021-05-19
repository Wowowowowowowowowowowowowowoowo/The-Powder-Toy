#include "ColorPicker.h"

#include <iomanip>

#include "misc.h" // for RGB_to_HSV
#include "common/Format.h"
#include "graphics/VideoBuffer.h"
#include "interface/Label.h"
#include "interface/Button.h"
#include "interface/Textbox.h"

ColorPicker::ColorPicker(ARGBColour initialColor, std::function<void (int)> callback):
	ui::Window(Point(CENTERED, CENTERED), Point(266, 175)),
	callback(callback)
{
	auto colorChange = [this] {
		int r, g, b, alpha;
		r = Format::StringToNumber<int>(rValue->GetText());
		g = Format::StringToNumber<int>(gValue->GetText());
		b = Format::StringToNumber<int>(bValue->GetText());
		alpha = Format::StringToNumber<int>(aValue->GetText());
		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;
		if (alpha > 255)
			alpha = 255;

		RGB_to_HSV(r, g, b, &currentHue, &currentSaturation, &currentValue);
		currentAlpha = alpha;
		UpdateTextboxes(r, g, b, alpha);
	};

	rValue = new Textbox(Point(5, size.Y-23), Point(30, 17), "255");
	rValue->SetCallback(colorChange);
	rValue->SetCharacterLimit(3);
	rValue->SetType(Textbox::NUMBER);
	this->AddComponent(rValue);

	gValue = new Textbox(Point(40, size.Y-23), Point(30, 17), "255");
	gValue->SetCallback(colorChange);
	gValue->SetCharacterLimit(3);
	gValue->SetType(Textbox::NUMBER);
	this->AddComponent(gValue);

	bValue = new Textbox(Point(75, size.Y-23), Point(30, 17), "255");
	bValue->SetCallback(colorChange);
	bValue->SetCharacterLimit(3);
	bValue->SetType(Textbox::NUMBER);
	this->AddComponent(bValue);

	aValue = new Textbox(Point(110, size.Y-23), Point(30, 17), "255");
	aValue->SetCallback(colorChange);
	aValue->SetCharacterLimit(3);
	aValue->SetType(Textbox::NUMBER);
	this->AddComponent(aValue);

	hexValue = new Label(Point(150, size.Y-23), Point(53, 17), "0xFFFFFFFF");
	this->AddComponent(hexValue);

	Button * doneButton = new Button(Point(size.X-45, size.Y-23), Point(40, 17), "Done");
	doneButton->SetCallback({ [this] (int mb) {
		if (this->callback)
		{
			int Red = Format::StringToNumber<int>(rValue->GetText());
			int Green = Format::StringToNumber<int>(gValue->GetText());
			int Blue = Format::StringToNumber<int>(bValue->GetText());
			ARGBColour col = COLARGB(currentAlpha, Red, Green, Blue);
			this->callback(col);
		}
	} });
	doneButton->SetCloseButton(true);
	this->AddComponent(doneButton);

	RGB_to_HSV(COLR(initialColor), COLG(initialColor), COLB(initialColor), &currentHue, &currentSaturation, &currentValue);
	currentAlpha = COLA(initialColor);
	UpdateTextboxes(COLR(initialColor), COLG(initialColor), COLB(initialColor), COLA(initialColor));
}

void ColorPicker::UpdateTextboxes(int r, int g, int b, int a)
{
	rValue->SetText(Format::NumberToString<int>(r));
	gValue->SetText(Format::NumberToString<int>(g));
	bValue->SetText(Format::NumberToString<int>(b));
	aValue->SetText(Format::NumberToString<int>(a));
	std::stringstream hex;
	hex << std::hex << std::uppercase << std::setfill('0') << std::setw(2);
	hex << a << r << g << b;
	hexValue->SetText(hex.str());
}

void ColorPicker::OnDraw(gfx::VideoBuffer *buf)
{
	buf->DrawRect(4, 4, 258, 130, 180, 180, 180, 255);

	buf->DrawRect(4, 4+4+128, 258, 12, 180, 180, 180, 255);


	int offsetX = 5;
	int offsetY = 5;


	//draw color square
	int lastx = -1, currx = 0;
	for(int saturation = 0; saturation <= 255; saturation+=2)
	{
		for(int hue = 0; hue <= 359; hue++)
		{
			currx = clamp_flt(float(hue), 0, 359)+offsetX;
			if (currx == lastx)
				continue;
			lastx = currx;
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(hue, 255-saturation, currentValue, &cr, &cg, &cb);
			buf->DrawPixel(currx, (saturation/2)+offsetY, cr, cg, cb, currentAlpha);
		}
	}

	//draw brightness bar
	for(int value = 0; value <= 255; value++)
		for(int i = 0;  i < 10; i++)
		{
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(currentHue, currentSaturation, value, &cr, &cg, &cb);

			buf->DrawPixel(value+offsetX, i+offsetY+127+5, cr, cg, cb, currentAlpha);
		}

	//draw color square pointer
	int currentHueX = clamp_flt(float(currentHue), 0, 359);
	int currentSaturationY = ((255-currentSaturation)/2);
	buf->XorLine(offsetX+currentHueX, offsetY+currentSaturationY-5, offsetX+currentHueX, offsetY+currentSaturationY-1);
	buf->XorLine(offsetX+currentHueX, offsetY+currentSaturationY+1, offsetX+currentHueX, offsetY+currentSaturationY+5);
	buf->XorLine(offsetX+currentHueX-5, offsetY+currentSaturationY, offsetX+currentHueX-1, offsetY+currentSaturationY);
	buf->XorLine(offsetX+currentHueX+1, offsetY+currentSaturationY, offsetX+currentHueX+5, offsetY+currentSaturationY);

	//draw brightness bar pointer
	int currentValueX = int(restrict_flt(float(currentValue), 0, 254));
	buf->XorLine(offsetX+currentValueX, offsetY+4+128, offsetX+currentValueX, offsetY+13+128);
	buf->XorLine(offsetX+currentValueX+1, offsetY+4+128, offsetX+currentValueX+1, offsetY+13+128);
}

void ColorPicker::OnMouseMove(int x, int y, Point difference)
{
	if (mouseDown)
	{
		x -= position.X + 5;
		y -= position.Y + 5;

		currentHue = int((float(x) / float(255)) * 359.0f);
		currentSaturation = 255 - (y * 2);

		if (currentSaturation > 255)
			currentSaturation = 255;
		if (currentSaturation < 0)
			currentSaturation = 0;
		if (currentHue > 359)
			currentHue = 359;
		if (currentHue < 0)
			currentHue = 0;
	}

	if (valueMouseDown)
	{
		x -= position.X + 5;
		//y -= position.Y + 5;

		currentValue = x;

		if (currentValue > 255)
			currentValue = 255;
		if (currentValue < 0)
			currentValue = 0;
	}

	if (mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}
}

void ColorPicker::OnMouseDown(int x, int y, unsigned char button)
{
	x -= position.X + 5;
	y -= position.Y + 5;
	if (x >= 0 && x < 256 && y >= 0 && y <= 128)
	{
		mouseDown = true;
		currentHue = int((float(x) / float(255)) * 359.0f);
		currentSaturation = 255 - (y * 2);

		if(currentSaturation > 255)
			currentSaturation = 255;
		if(currentSaturation < 0)
			currentSaturation = 0;
		if(currentHue > 359)
			currentHue = 359;
		if(currentHue < 0)
			currentHue = 0;
	}

	if (x >= 0 && x < 256 && y >= 132 && y <= 142)
	{
		valueMouseDown = true;
		currentValue = x;

		if(currentValue > 255)
			currentValue = 255;
		if(currentValue < 0)
			currentValue = 0;
	}

	if (mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}
}

void ColorPicker::OnMouseUp(int x, int y, unsigned char button)
{
	if (mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}

	if (mouseDown)
	{
		mouseDown = false;
		x -= position.X + 5;
		y -= position.Y + 5;

		currentHue = int((float(x) / float(255)) * 359.0f);
		currentSaturation = 255 - (y * 2);

		if(currentSaturation > 255)
			currentSaturation = 255;
		if(currentSaturation < 0)
			currentSaturation = 0;
		if(currentHue > 359)
			currentHue = 359;
		if(currentHue < 0)
			currentHue = 0;
	}

	if (valueMouseDown)
	{
		valueMouseDown = false;

		x -= position.X + 5;
		//y -= position.Y + 5;

		currentValue = x;

		if (currentValue > 255)
			currentValue = 255;
		if (currentValue < 0)
			currentValue = 0;
	}
}

void ColorPicker::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (key == SDLK_RETURN)
	{
		this->toDelete = true;
	}
	else if (key == SDLK_TAB)
	{
		if (rValue->IsFocused())
		{
			FocusComponent(gValue);
			gValue->SelectAll();
		}
		else if (gValue->IsFocused())
		{
			FocusComponent(bValue);
			bValue->SelectAll();
		}
		else if (bValue->IsFocused())
		{
			FocusComponent(aValue);
			aValue->SelectAll();
		}
		else if (aValue->IsFocused())
		{
			FocusComponent(rValue);
			rValue->SelectAll();
		}
	}
}
