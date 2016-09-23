#include "RenderModesUI.h"
#include "defines.h"
#include "graphics.h"
#include "powdergraphics.h"
#include "common/Point.h"
#include "game/ToolTip.h"
#include "graphics/Renderer.h"
#include "graphics/VideoBuffer.h"
#include "interface/Label.h"
#include "interface/Button.h"
#include "interface/Checkbox.h"


RenderModesUI::RenderModesUI():
	Window_(Point(0, YRES), Point(XRES, MENUSIZE)),
	last_render_mode(render_mode),
	last_display_mode(display_mode),
	last_color_mode(Renderer::Ref().GetColorMode()),
	renderCheckboxes(std::vector<std::pair<Checkbox*, unsigned int> >()),
	displayCheckboxes(std::vector<std::pair<Checkbox*, unsigned int> >()),
	colorCheckboxes(std::vector<std::pair<Checkbox*, unsigned int> >())
{
	InitializeCheckboxes();

}

class ToggleRenderModeAction : public CheckboxAction
{
	int render_md;
public:
	ToggleRenderModeAction(unsigned int render_md):
		render_md(render_md)
	{

	}

	virtual void CheckboxActionCallback(Checkbox *checkbox, unsigned char b)
	{
		if (checkbox->IsChecked())
			Renderer::Ref().AddRenderMode(render_md);
		else
			Renderer::Ref().RemoveRenderMode(render_md);
		render_mode = Renderer::Ref().GetRenderModesRaw();
	}
};

void RenderModesUI::InitializeRenderCheckbox(Checkbox *checkbox, unsigned int mode)
{
	checkbox->SetCallback(new ToggleRenderModeAction(mode));
	if (Renderer::Ref().HasRenderMode(mode))
		checkbox->SetChecked(true);
	this->AddComponent(checkbox);
	renderCheckboxes.push_back(std::pair<Checkbox*, unsigned int>(checkbox, mode));
}

class ToggleDisplayModeAction : public CheckboxAction
{
	int display_md;
public:
	ToggleDisplayModeAction(unsigned int display_md):
		display_md(display_md)
	{

	}

	virtual void CheckboxActionCallback(Checkbox *checkbox, unsigned char b)
	{
		if (checkbox->IsChecked())
			Renderer::Ref().AddDisplayMode(display_md);
		else
			Renderer::Ref().RemoveDisplayMode(display_md);
		display_mode = Renderer::Ref().GetDisplayModesRaw();
	}
};

void RenderModesUI::InitializeDisplayCheckbox(Checkbox *checkbox, unsigned int mode)
{
	checkbox->SetCallback(new ToggleDisplayModeAction(mode));
	if (Renderer::Ref().HasDisplayMode(mode))
		checkbox->SetChecked(true);
	this->AddComponent(checkbox);
	displayCheckboxes.push_back(std::pair<Checkbox*, unsigned int>(checkbox, mode));
}

class ToggleColorModeAction : public CheckboxAction
{
	int color_md;
public:
	ToggleColorModeAction(unsigned int color_md):
		color_md(color_md)
	{

	}

	virtual void CheckboxActionCallback(Checkbox *checkbox, unsigned char b)
	{
		if (checkbox->IsChecked())
			Renderer::Ref().SetColorMode(color_md);
		else
			Renderer::Ref().SetColorMode(COLOR_DEFAULT);
	}
};

void RenderModesUI::InitializeColorCheckbox(Checkbox *checkbox, unsigned int mode)
{
	checkbox->SetCallback(new ToggleColorModeAction(mode));
	if (Renderer::Ref().GetColorMode() == mode)
		checkbox->SetChecked(true);
	this->AddComponent(checkbox);
	colorCheckboxes.push_back(std::pair<Checkbox*, unsigned int>(checkbox, mode));
}

void RenderModesUI::SetCheckboxToolTip(Checkbox *checkbox, std::string tooltip)
{
	checkbox->SetTooltip(new ToolTip(tooltip, Point(16, YRES-24), TOOLTIP, 255));
}

void RenderModesUI::InitializeCheckboxes()
{
	Checkbox *effectsCheckbox = new Checkbox(Point(3, 3), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xE1");
	InitializeRenderCheckbox(effectsCheckbox, RENDER_EFFE);
	SetCheckboxToolTip(effectsCheckbox, "Adds Special flare effects to some elements");

	Checkbox *glowCheckbox = new Checkbox(effectsCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDF");
	InitializeRenderCheckbox(glowCheckbox, RENDER_GLOW);
	SetCheckboxToolTip(glowCheckbox, "Glow effect on some elements");

	Checkbox *fireCheckbox = new Checkbox(effectsCheckbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\x9B");
	InitializeRenderCheckbox(fireCheckbox, RENDER_FIRE);
	SetCheckboxToolTip(fireCheckbox, "Fire effect for gasses");

	Checkbox *blurCheckbox = new Checkbox(fireCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xC4");
	InitializeRenderCheckbox(blurCheckbox, RENDER_BLUR);
	SetCheckboxToolTip(blurCheckbox, "Blur effect for liquids");

	Checkbox *basicCheckbox = new Checkbox(fireCheckbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDB");
	InitializeRenderCheckbox(basicCheckbox, RENDER_BASC);
	SetCheckboxToolTip(basicCheckbox, "Basic rendering, without this, most things will be invisible");

	Checkbox *blobCheckbox = new Checkbox(basicCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xBF");
	InitializeRenderCheckbox(blobCheckbox, RENDER_BLOB);
	SetCheckboxToolTip(blobCheckbox, "Makes everything be drawn like a blob");

	Checkbox *sparkCheckbox = new Checkbox(basicCheckbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDF");
	InitializeRenderCheckbox(sparkCheckbox, RENDER_SPRK);
	SetCheckboxToolTip(sparkCheckbox, "Glow effect on sparks");

	Checkbox *noneCheckbox = new Checkbox(sparkCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDB");
	InitializeRenderCheckbox(noneCheckbox, RENDER_NONE);
	SetCheckboxToolTip(noneCheckbox, "Even more basic rendering - only elements and stickmen are rendered");

	line1Pos = sparkCheckbox->Right(Point(4, 0)).X;

	Checkbox *crackerCheckbox = new Checkbox(Point(line1Pos+6, sparkCheckbox->GetPosition().Y), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xD4");
	InitializeDisplayCheckbox(crackerCheckbox, DISPLAY_AIRC);
	SetCheckboxToolTip(crackerCheckbox, "Displays pressure as red and blue, and velocity as white");

	Checkbox *airPressureCheckbox = new Checkbox(crackerCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\x99");
	InitializeDisplayCheckbox(airPressureCheckbox, DISPLAY_AIRP);
	SetCheckboxToolTip(airPressureCheckbox, "Displays pressure, red is positive and blue is negative");

	Checkbox *airVelocityCheckbox = new Checkbox(crackerCheckbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\x98");
	InitializeDisplayCheckbox(airVelocityCheckbox, DISPLAY_AIRV);
	SetCheckboxToolTip(airVelocityCheckbox, "Displays velocity and positive pressure: up/down adds blue, right/left adds red, still pressure adds green");

	Checkbox *airHeatCheckbox = new Checkbox(airVelocityCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xBD");
	InitializeDisplayCheckbox(airHeatCheckbox, DISPLAY_AIRH);
	SetCheckboxToolTip(airHeatCheckbox, "Displays the temperature of the air like heat display does");

	Checkbox *warpCheckbox = new Checkbox(airVelocityCheckbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDE");
	InitializeDisplayCheckbox(warpCheckbox, DISPLAY_WARP);
	SetCheckboxToolTip(warpCheckbox, "Gravity lensing, Newtonian Gravity bends light with this on");

	Checkbox *persistentCheckbox = new Checkbox(warpCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\x9A");
	InitializeDisplayCheckbox(persistentCheckbox, DISPLAY_PERS);
	SetCheckboxToolTip(persistentCheckbox, "Element paths persist on the screen for a while");

	line2Pos = persistentCheckbox->Right(Point(4, 0)).X;

	Checkbox *basic2Checkbox = new Checkbox(Point(line2Pos+6, warpCheckbox->GetPosition().Y), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xDB");
	InitializeColorCheckbox(basic2Checkbox, COLOR_BASC);
	SetCheckboxToolTip(basic2Checkbox, "No special effects at all for anything, overrides all other options and deco");

	Checkbox *lifeCheckbox = new Checkbox(basic2Checkbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xE0");
	InitializeColorCheckbox(lifeCheckbox, COLOR_LIFE);
	SetCheckboxToolTip(lifeCheckbox, "Displays the life value of elements in greyscale gradients");

	Checkbox *heatCheckbox = new Checkbox(basic2Checkbox->Right(Point(4, 0)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xBD");
	InitializeColorCheckbox(heatCheckbox, COLOR_HEAT);
	SetCheckboxToolTip(heatCheckbox, "Displays temperatures of the elements, dark blue is coldest, pink is hottest");

	Checkbox *heatGradientCheckbox = new Checkbox(heatCheckbox->Below(Point(0, 2)), Point(Checkbox::AUTOSIZE, Checkbox::AUTOSIZE), "\xD3");
	InitializeColorCheckbox(heatGradientCheckbox, COLOR_GRAD);
	SetCheckboxToolTip(heatGradientCheckbox, "Changes colors of elements slightly to show heat diffusing through them");
}

void RenderModesUI::OnTick(uint32_t ticks)
{
	if (render_mode != last_render_mode)
	{
		for (std::vector<std::pair<Checkbox*, unsigned int> >::iterator iter = renderCheckboxes.begin(), end = renderCheckboxes.end(); iter != end; iter++)
		{
			unsigned int mode = (*iter).second;
			if (Renderer::Ref().HasRenderMode(mode))
				(*iter).first->SetChecked(true);
			else
				(*iter).first->SetChecked(false);
		}
		last_render_mode = render_mode;
	}

	if (display_mode != last_display_mode)
	{
		for (std::vector<std::pair<Checkbox*, unsigned int> >::iterator iter = displayCheckboxes.begin(), end = displayCheckboxes.end(); iter != end; iter++)
		{
			unsigned int mode = (*iter).second;
			if (Renderer::Ref().HasDisplayMode(mode))
				(*iter).first->SetChecked(true);
			else
				(*iter).first->SetChecked(false);
		}
		last_display_mode = display_mode;
	}

	if (Renderer::Ref().GetColorMode() != last_color_mode)
	{
		last_color_mode = Renderer::Ref().GetColorMode();
		for (std::vector<std::pair<Checkbox*, unsigned int> >::iterator iter = colorCheckboxes.begin(), end = colorCheckboxes.end(); iter != end; iter++)
		{
			unsigned int mode = (*iter).second;
			if (last_color_mode == mode)
				(*iter).first->SetChecked(true);
			else
				(*iter).first->SetChecked(false);
		}
	}
}

void RenderModesUI::OnDraw(VideoBuffer *buf)
{
	buf->DrawLine(line1Pos, 0, line1Pos, MENUSIZE, 255, 255, 255, 255);
	buf->DrawLine(line2Pos, 0, line2Pos, MENUSIZE, 255, 255, 255, 255);
}

void RenderModesUI::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (key == SDLK_RETURN)
		this->toDelete = true;
}
