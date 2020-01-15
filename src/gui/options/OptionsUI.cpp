#include <iostream>
#ifdef WIN
#include <direct.h>
#define getcwd _getcwd
#endif
#if defined(LIN) || defined(MACOSX)
#include <unistd.h> // getcwd
#endif

#include "OptionsUI.h"
#include "common/tpt-minmax.h"
#include "game/Menus.h"
#include "graphics/VideoBuffer.h"
#include "interface/Button.h"
#include "interface/Checkbox.h"
#include "interface/Dropdown.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "interface/ScrollWindow.h"
#include "interface/Textbox.h"
#include "simulation/Simulation.h"

OptionsUI::OptionsUI(Simulation *sim):
	ui::Window(Point(CENTERED, CENTERED), Point(310, 350)),
	sim(sim),
	oldEdgeMode(sim->GetEdgeMode())
{
#ifndef TOUCHUI
	int checkboxHeight = 13;
	bool useCheckIcon = true;
	int okButtonHeight = 15;
#else
	int checkboxHeight = 20;
	bool useCheckIcon = false;
	int okButtonHeight = 25;
#endif

	Label *headerLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Options");
	headerLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(headerLabel);

	int headerHeight = headerLabel->GetPosition().Y + headerLabel->GetSize().Y;
	scrollArea = new ui::ScrollWindow(Point(0, headerHeight), this->size - Point(0, headerHeight + okButtonHeight));
	scrollArea->SetOnDraw([this](gfx::VideoBuffer *buf) { OnSubwindowDraw(buf); });
	this->AddSubwindow(scrollArea);

	Component *prev;

	prev = heatSimCheckbox = new Checkbox(Point(5, 4), Point(Checkbox::AUTOSIZE, checkboxHeight), "Heat simulation");
	heatSimCheckbox->UseCheckIcon(useCheckIcon);
	heatSimCheckbox->SetCallback([&](bool checked) { this->HeatSimChecked(checked); });
	scrollArea->AddComponent(heatSimCheckbox);

	Label *descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Causes unexpected behavior when disabled");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

	prev = ambientCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Ambient heat simulation");
	ambientCheckbox->UseCheckIcon(useCheckIcon);
	ambientCheckbox->SetCallback([&](bool checked) { this->AmbientChecked(checked); });
	scrollArea->AddComponent(ambientCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Heat transfers through empty space");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

	prev = newtonianCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Newtonian Gravity");
	newtonianCheckbox->UseCheckIcon(useCheckIcon);
	newtonianCheckbox->SetCallback([&](bool checked) { this->NewtonianChecked(checked); });
	scrollArea->AddComponent(newtonianCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Simulate local gravity fields");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

#ifdef TOUCHUI
	prev = decorationCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Decorations");
	decorationCheckbox->UseCheckIcon(useCheckIcon);
	decorationCheckbox->SetCallback([&](bool checked) { this->DecorationsChecked(checked); });
	scrollArea->AddComponent(decorationCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Show deco color on elements");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);
#endif

	prev = waterEqalizationCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Water Equalization");
	waterEqalizationCheckbox->UseCheckIcon(useCheckIcon);
	waterEqalizationCheckbox->SetCallback([&](bool checked) { this->WaterEqualizationChecked(checked); });
	scrollArea->AddComponent(waterEqalizationCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Water equalizes in a U-shaped pipe (lags game)");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);


	prev = airSimDropdown = new Dropdown(prev->Below(Point(0, 17)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"On", "Pressure Off", "Velocity Off", "Off", "No Update"});
	airSimDropdown->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - airSimDropdown->GetSize().X, airSimDropdown->GetPosition().Y));
	airSimDropdown->SetCallback([&](unsigned int option) { this->AirSimSelected(option); });
	scrollArea->AddComponent(airSimDropdown);

	descLabel = new Label(Point(17, prev->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Air Simulation Mode:");
	scrollArea->AddComponent(descLabel);

	prev = gravityDropdown = new Dropdown(prev->Below(Point(0, 4)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"Vertical", "Off", "Radial"});
	gravityDropdown->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - gravityDropdown->GetSize().X, gravityDropdown->GetPosition().Y));
	gravityDropdown->SetCallback([&](unsigned int option) { this->GravitySelected(option); });
	scrollArea->AddComponent(gravityDropdown);

	descLabel = new Label(Point(17, prev->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Gravity Simulation Mode:");
	scrollArea->AddComponent(descLabel);

	prev = edgeModeDropdown = new Dropdown(prev->Below(Point(0, 4)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"Void", "Solid", "Loop"});
	edgeModeDropdown->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - edgeModeDropdown->GetSize().X, edgeModeDropdown->GetPosition().Y));
	edgeModeDropdown->SetCallback([&](unsigned int option) { this->EdgeModeSelected(option); });
	scrollArea->AddComponent(edgeModeDropdown);

	descLabel = new Label(Point(17, prev->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Edge Mode:");
	scrollArea->AddComponent(descLabel);

	prev = decoSpaceDropdown = new Dropdown(prev->Below(Point(0, 4)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"sRGB", "Linear", "Gamma 2.2", "Gamma 1.8"});
	decoSpaceDropdown->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - decoSpaceDropdown->GetSize().X, decoSpaceDropdown->GetPosition().Y));
	decoSpaceDropdown->SetCallback([&](unsigned int option) { this->DecoSpaceSelected(option); });
	scrollArea->AddComponent(decoSpaceDropdown);

	descLabel = new Label(Point(17, prev->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Smudge Tool Color Space:");
	scrollArea->AddComponent(descLabel);

	// set dropdown widths to width of largest one
	int maxWidth = airSimDropdown->GetSize().X;
	maxWidth = tpt::max(maxWidth, gravityDropdown->GetSize().X);
	maxWidth = tpt::max(maxWidth, edgeModeDropdown->GetSize().X);
	maxWidth = tpt::max(maxWidth, decoSpaceDropdown->GetSize().X);
	int xPos = scrollArea->GetUsableWidth() - 5 - maxWidth;
	airSimDropdown->SetPosition(Point(xPos, airSimDropdown->GetPosition().Y));
	airSimDropdown->SetSize(Point(maxWidth, airSimDropdown->GetSize().Y));
	gravityDropdown->SetPosition(Point(xPos, gravityDropdown->GetPosition().Y));
	gravityDropdown->SetSize(Point(maxWidth, gravityDropdown->GetSize().Y));
	edgeModeDropdown->SetPosition(Point(xPos, edgeModeDropdown->GetPosition().Y));
	edgeModeDropdown->SetSize(Point(maxWidth, edgeModeDropdown->GetSize().Y));
	decoSpaceDropdown->SetPosition(Point(xPos, decoSpaceDropdown->GetPosition().Y));
	decoSpaceDropdown->SetSize(Point(maxWidth, decoSpaceDropdown->GetSize().Y));

#ifndef TOUCHUI
	prev = doubleSizeCheckbox  = new Checkbox(Point(heatSimCheckbox->GetPosition().X, prev->Below(Point(0, 12)).Y), Point(Checkbox::AUTOSIZE, checkboxHeight), "Large Window");
	doubleSizeCheckbox->UseCheckIcon(useCheckIcon);
	doubleSizeCheckbox->SetCallback([&](bool checked) { this->DoubleSizeChecked(checked); });
	scrollArea->AddComponent(doubleSizeCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Double window size for larger screens");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

	prev = resizableCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Resizable Window");
	resizableCheckbox->UseCheckIcon(useCheckIcon);
	resizableCheckbox->SetCallback([&](bool checked) { this->ResizableChecked(checked); });
	scrollArea->AddComponent(resizableCheckbox);

	resizableLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Allow resizing window");
	resizableLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(resizableLabel);

	forceIntegerScalingCheckbox = new Checkbox(Point(0, 0), Point(Checkbox::AUTOSIZE, checkboxHeight), "Force Integer Scaling");
	forceIntegerScalingCheckbox->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - forceIntegerScalingCheckbox->GetSize().X, resizableCheckbox->GetPosition().Y));
	forceIntegerScalingCheckbox->UseCheckIcon(useCheckIcon);
	forceIntegerScalingCheckbox->SetCallback([&](bool checked) { this->ForceIntegerScalingChecked(checked); });
	scrollArea->AddComponent(forceIntegerScalingCheckbox);

	forceIntegerScalingLabel = new Label(forceIntegerScalingCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Less Blurry");
	forceIntegerScalingLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(forceIntegerScalingLabel);

	filteringDropdown = new Dropdown(Point(0, 0), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"Nearest", "Linear", "Best"});
	filteringDropdown->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - filteringDropdown->GetSize().X, resizableCheckbox->GetPosition().Y));
	filteringDropdown->SetCallback([&](unsigned int option) { this->FilteringSelected(option); });
	scrollArea->AddComponent(filteringDropdown);
	filteringDropdown->SetVisible(false);

	filteringLabel = new Label(Point(0, 0), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Pixel sampling mode:");
	filteringLabel->SetPosition(Point(filteringDropdown->Left(Point(filteringLabel->GetSize().X + 5, 0)).X, filteringDropdown->GetPosition().Y));
	scrollArea->AddComponent(filteringLabel);
	filteringLabel->SetVisible(false);

	prev = fullscreenCheckbox = new Checkbox(prev->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Fullscreen");
	fullscreenCheckbox->UseCheckIcon(true);
	fullscreenCheckbox->SetCallback([&](bool checked) { this->FullscreenChecked(checked); });
	scrollArea->AddComponent(fullscreenCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Fill the entire screen");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

	altFullscreenCheckbox = new Checkbox(Point(0, 0), Point(Checkbox::AUTOSIZE, checkboxHeight), "Change Resolution");
	altFullscreenCheckbox->SetPosition(Point(scrollArea->GetUsableWidth() - 5 - altFullscreenCheckbox->GetSize().X, fullscreenCheckbox->GetPosition().Y));
	altFullscreenCheckbox->UseCheckIcon(useCheckIcon);
	altFullscreenCheckbox->SetCallback([&](bool checked) { this->AltFullscreenChecked(checked); });
	scrollArea->AddComponent(altFullscreenCheckbox);

	descLabel = new Label(altFullscreenCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Old fullscreen");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);


	prev = fastQuitCheckbox = new Checkbox(prev->Below(Point(0, 24)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Fast Quit");
	fastQuitCheckbox->UseCheckIcon(useCheckIcon);
	fastQuitCheckbox->SetCallback([&](bool checked) { this->FastQuitChecked(checked); });
	scrollArea->AddComponent(fastQuitCheckbox);

	descLabel = new Label(fastQuitCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Always exit completely when clicking \"X\"");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);
#endif

	prev = updatesCheckbox = new Checkbox(Point(heatSimCheckbox->GetPosition().X, prev->Below(Point(0, 15)).Y), Point(Checkbox::AUTOSIZE, checkboxHeight), "Update Check");
	updatesCheckbox->UseCheckIcon(useCheckIcon);
	updatesCheckbox->SetCallback([&](bool checked) { this->UpdatesChecked(checked); });
	scrollArea->AddComponent(updatesCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Check for updates at https://starcatcher.us/TPT");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

#ifndef TOUCHUI
	prev = stickyCategoriesCheckbox = new Checkbox(prev->Below(Point(0, 15)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Sticky categories");
	stickyCategoriesCheckbox->UseCheckIcon(useCheckIcon);
	stickyCategoriesCheckbox->SetCallback([&](bool checked) { this->StickyCatsChecked(checked); });
	scrollArea->AddComponent(stickyCategoriesCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Switch between menusections by clicking");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);
#endif

	prev = savePressureCheckbox = new Checkbox(prev->Below(Point(0, 15)), Point(Checkbox::AUTOSIZE, checkboxHeight), "Include Pressure");
	savePressureCheckbox->UseCheckIcon(useCheckIcon);
	savePressureCheckbox->SetCallback([&](bool checked) { this->SavePressureChecked(checked); });
	scrollArea->AddComponent(savePressureCheckbox);

	descLabel = new Label(prev->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Include pressure in saves and stamps");
	descLabel->SetColor(COLRGB(150, 150, 150));
	scrollArea->AddComponent(descLabel);

#ifndef TOUCHUI
	prev = dataFolderButton = new Button(prev->Below(Point(0, 17)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "Open Data Folder");
	dataFolderButton->SetCallback([&](int mb) { this->DataFolderClicked(); });
	scrollArea->AddComponent(dataFolderButton);

	scrollArea->SetScrollSize(prev->Below(Point(0, 5)).Y);
#else
	scrollArea->SetScrollSize(descLabel->Below(Point(0, 5)).Y);
#endif

#ifndef TOUCHUI
	Button *okButton = new Button(Point(0, this->size.Y - okButtonHeight), Point(this->size.X+1, okButtonHeight), "OK");
#else
	Button *okButton = new Button(Point(0, this->size.Y - okButtonHeight), Point(this->size.X+1, okButtonHeight), "OK");
#endif
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);

	InitializeOptions();
}

void OptionsUI::InitializeOptions()
{
	heatSimCheckbox->SetChecked(!legacy_enable);
	ambientCheckbox->SetChecked(aheat_enable);
	newtonianCheckbox->SetChecked(sim->grav->IsEnabled());
	waterEqalizationCheckbox->SetChecked(water_equal_test);

	airSimDropdown->SetSelectedOption(airMode);
	gravityDropdown->SetSelectedOption(sim->gravityMode);
	edgeModeDropdown->SetSelectedOption(sim->edgeMode);
	decoSpaceDropdown->SetSelectedOption(sim->decoSpace);

#ifdef TOUCHUI
	decorationCheckbox->SetChecked(decorations_enable);
#else
	doubleSizeCheckbox->SetChecked(Engine::Ref().GetScale() >= 2);
	resizableCheckbox->SetChecked(Engine::Ref().IsResizable());
	filteringDropdown->SetSelectedOption(Engine::Ref().GetPixelFilteringMode());
	filteringDropdown->SetEnabled(resizableCheckbox->IsChecked());
	fullscreenCheckbox->SetChecked(Engine::Ref().IsFullscreen());
	altFullscreenCheckbox->SetChecked(Engine::Ref().IsAltFullscreen());
	forceIntegerScalingCheckbox->SetChecked(Engine::Ref().IsForceIntegerScaling());
#endif

#ifndef TOUCHUI
	fastQuitCheckbox->SetChecked(Engine::Ref().IsFastQuit());
	stickyCategoriesCheckbox->SetChecked(stickyCategories);
#endif
	updatesCheckbox->SetChecked(doUpdates);
	savePressureCheckbox->SetChecked(sim->includePressure);
}

void OptionsUI::HeatSimChecked(bool checked)
{
	legacy_enable = !legacy_enable;
}

void OptionsUI::AmbientChecked(bool checked)
{
	aheat_enable = checked;
}

void OptionsUI::NewtonianChecked(bool checked)
{
	if (checked)
		sim->grav->StartAsync();
	else
		sim->grav->StopAsync();
}

void OptionsUI::DecorationsChecked(bool checked)
{
	decorations_enable = checked;
}

void OptionsUI::WaterEqualizationChecked(bool checked)
{
	water_equal_test = checked;
}

void OptionsUI::AirSimSelected(unsigned int option)
{
	airMode = option;
}

void OptionsUI::GravitySelected(unsigned int option)
{
	sim->gravityMode = option;
}

void OptionsUI::EdgeModeSelected(unsigned int option)
{
	unsigned int edgeMode = option;
	if (edgeMode == 1 && oldEdgeMode != 1)
		draw_bframe();
	else if (edgeMode != 1 && oldEdgeMode == 1)
		erase_bframe();
	if (edgeMode != oldEdgeMode)
	{
		sim->edgeMode = edgeMode;
		sim->saveEdgeMode = -1;
	}
	oldEdgeMode = sim->GetEdgeMode();
}


void OptionsUI::DecoSpaceSelected(unsigned int option)
{
	sim->decoSpace = option;
}


void OptionsUI::DoubleSizeChecked(bool checked)
{
	Engine::Ref().SetScale(checked ? 2 : 1);
}

void OptionsUI::ResizableChecked(bool checked)
{
	Engine::Ref().SetResizable(checked, true);
	filteringDropdown->SetEnabled(checked);
}

void OptionsUI::FilteringSelected(unsigned int option)
{
	Engine::Ref().SetPixelFilteringMode(option, true);
}

void OptionsUI::FullscreenChecked(bool checked)
{
	Engine::Ref().SetFullscreen(checked);
}

void OptionsUI::AltFullscreenChecked(bool checked)
{
	Engine::Ref().SetAltFullscreen(checked);
}

void OptionsUI::ForceIntegerScalingChecked(bool checked)
{
	Engine::Ref().SetForceIntegerScaling(checked);
}

void OptionsUI::FastQuitChecked(bool checked)
{
	Engine::Ref().SetFastQuit(checked);
}

void OptionsUI::UpdatesChecked(bool checked)
{
	doUpdates = checked;
}

void OptionsUI::StickyCatsChecked(bool checked)
{
	stickyCategories = checked;
}

void OptionsUI::SavePressureChecked(bool checked)
{
	sim->includePressure = checked;
}

void OptionsUI::DataFolderClicked()
{
#ifdef WIN
	const char *openCommand = "explorer ";
#elif MACOSX
	const char *openCommand = "open ";
//#elif LIN
#else
	const char *openCommand = "xdg-open ";
#endif
	char *workingDirectory = new char[FILENAME_MAX + strlen(openCommand)];
	sprintf(workingDirectory, "%s\"%s\"", openCommand, getcwd(NULL, 0));
	int ret = system(workingDirectory);
	if (ret)
		std::cout << "Error, could not open data directory" << std::endl;
	delete[] workingDirectory;
}

void OptionsUI::OnDraw(gfx::VideoBuffer *buf)
{
	buf->DrawLine(0, scrollArea->GetPosition().Y - 1, size.X, scrollArea->GetPosition().Y - 1, 200, 200, 200, 255);

#ifndef TOUCHUI
	if (filteringDropdown->IsSelectingOption())
	{
		resizableLabel->SetText("These options make TPT appear extremely blurry");
		resizableLabel->SetColor(COLRGB(255, 0, 0));
	}
	else
	{
		resizableLabel->SetText("Allow resizing window");
		resizableLabel->SetColor(COLRGB(150, 150, 150));
	}
#endif
}

void OptionsUI::OnSubwindowDraw(gfx::VideoBuffer *buf)
{
	buf->DrawLine(0, decoSpaceDropdown->Below(Point(0, 5)).Y, scrollArea->GetUsableWidth() - 1, decoSpaceDropdown->Below(Point(0, 5)).Y, 200, 200, 200, 255);
#ifndef TOUCHUI
	buf->DrawLine(0, altFullscreenCheckbox->Below(Point(0, 17)).Y, scrollArea->GetUsableWidth() - 1, altFullscreenCheckbox->Below(Point(0, 17)).Y, 200, 200, 200, 255);
#endif
}

void OptionsUI::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
#ifndef TOUCHUI
	if ((codeStep == 0 || codeStep == 1) && key == SDLK_UP)
		codeStep++;
	else if ((codeStep == 2 || codeStep == 3) && key == SDLK_DOWN)
		codeStep++;
	else if ((codeStep == 4 || codeStep == 6) && key == SDLK_LEFT)
		codeStep++;
	else if ((codeStep == 5 || codeStep == 7) && key == SDLK_RIGHT)
		codeStep++;
	else
		codeStep = 0;
	if (codeStep == 8)
	{
		filteringDropdown->SetVisible(true);
		filteringLabel->SetVisible(true);

		forceIntegerScalingCheckbox->SetVisible(false);
		forceIntegerScalingLabel->SetVisible(false);
	}
#endif
}
