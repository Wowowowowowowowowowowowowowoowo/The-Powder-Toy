#include <iostream>
#include <unistd.h> // getcwd

#include "OptionsUI.h"
#include "gravity.h"
#include "graphics/VideoBuffer.h"
#include "interface/Button.h"
#include "interface/Checkbox.h"
#include "interface/Dropdown.h"
#include "interface/Engine.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "simulation/Simulation.h"

OptionsUI::OptionsUI(Simulation *sim):
	Window_(Point(CENTERED, CENTERED), Point(255, 400)),
	sim(sim),
	oldEdgeMode(sim->GetEdgeMode())
{
	Label *headerLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Options");
	headerLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(headerLabel);

	heatSimCheckbox = new Checkbox(Point(5, 20), Point(Checkbox::AUTOSIZE, 13), "Heat simulation");
	heatSimCheckbox->UseCheckIcon(true);
	heatSimCheckbox->SetCallback([&](bool checked) { this->HeatSimChecked(checked); });
	this->AddComponent(heatSimCheckbox);

	Label *descLabel = new Label(heatSimCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Causes unexpected behavior when disabled");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	ambientCheckbox = new Checkbox(heatSimCheckbox->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, 13), "Ambient heat simulation");
	ambientCheckbox->UseCheckIcon(true);
	ambientCheckbox->SetCallback([&](bool checked) { this->AmbientChecked(checked); });
	this->AddComponent(ambientCheckbox);

	descLabel = new Label(ambientCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Heat transfers through empty space");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	newtonianCheckbox = new Checkbox(ambientCheckbox->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, 13), "Newtonian Gravity");
	newtonianCheckbox->UseCheckIcon(true);
	newtonianCheckbox->SetCallback([&](bool checked) { this->NewtonianChecked(checked); });
	this->AddComponent(newtonianCheckbox);

	descLabel = new Label(newtonianCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Simulate local gravity fields");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	waterEqalizationCheckbox = new Checkbox(newtonianCheckbox->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, 13), "Water Equalization");
	waterEqalizationCheckbox->UseCheckIcon(true);
	waterEqalizationCheckbox->SetCallback([&](bool checked) { this->WaterEqualizationChecked(checked); });
	this->AddComponent(waterEqalizationCheckbox);

	descLabel = new Label(waterEqalizationCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Water equalizes in a U-shaped pipe (lags game)");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);


	airSimDropdown = new Dropdown(waterEqalizationCheckbox->Below(Point(0, 17)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"On", "Pressure Off", "Velocity Off", "Off", "No Update"});
	airSimDropdown->SetPosition(Point(size.X - 5 - airSimDropdown->GetSize().X, airSimDropdown->GetPosition().Y));
	airSimDropdown->SetCallback([&](unsigned int option) { this->AirSimSelected(option); });
	this->AddComponent(airSimDropdown);

	descLabel = new Label(Point(17, airSimDropdown->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Air Simulation Mode:");
	this->AddComponent(descLabel);

	gravityDropdown = new Dropdown(airSimDropdown->Below(Point(0, 4)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"Vertical", "Off", "Radial"});
	gravityDropdown->SetPosition(Point(size.X - 5 - gravityDropdown->GetSize().X, gravityDropdown->GetPosition().Y));
	gravityDropdown->SetCallback([&](unsigned int option) { this->GravitySelected(option); });
	this->AddComponent(gravityDropdown);

	descLabel = new Label(Point(17, gravityDropdown->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Gravity Simulation Mode:");
	this->AddComponent(descLabel);

	edgeModeDropdown = new Dropdown(gravityDropdown->Below(Point(0, 4)), Point(Dropdown::AUTOSIZE, Dropdown::AUTOSIZE), {"Void", "Solid", "Loop"});
	edgeModeDropdown->SetPosition(Point(size.X - 5 - edgeModeDropdown->GetSize().X, edgeModeDropdown->GetPosition().Y));
	edgeModeDropdown->SetCallback([&](unsigned int option) { this->EdgeModeSelected(option); });
	this->AddComponent(edgeModeDropdown);

	descLabel = new Label(Point(17, edgeModeDropdown->GetPosition().Y), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Edge Mode:");
	this->AddComponent(descLabel);

	// set dropdown widths to width of largest one
	int maxWidth = airSimDropdown->GetSize().X;
	maxWidth = std::max(maxWidth, gravityDropdown->GetSize().X);
	maxWidth = std::max(maxWidth, edgeModeDropdown->GetSize().X);
	int xPos = size.X - 5 - maxWidth;
	airSimDropdown->SetPosition(Point(xPos, airSimDropdown->GetPosition().Y));
	airSimDropdown->SetSize(Point(maxWidth, airSimDropdown->GetSize().Y));
	gravityDropdown->SetPosition(Point(xPos, gravityDropdown->GetPosition().Y));
	gravityDropdown->SetSize(Point(maxWidth, gravityDropdown->GetSize().Y));
	edgeModeDropdown->SetPosition(Point(xPos, edgeModeDropdown->GetPosition().Y));
	edgeModeDropdown->SetSize(Point(maxWidth, edgeModeDropdown->GetSize().Y));


	doubleSizeCheckbox  = new Checkbox(Point(heatSimCheckbox->GetPosition().X, edgeModeDropdown->Below(Point(0, 12)).Y), Point(Checkbox::AUTOSIZE, 13), "Large Window");
	doubleSizeCheckbox->UseCheckIcon(true);
	doubleSizeCheckbox->SetCallback([&](bool checked) { this->DoubleSizeChecked(checked); });
	this->AddComponent(doubleSizeCheckbox);

	descLabel = new Label(doubleSizeCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Double window size for larger screens");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	resizableCheckbox = new Checkbox(doubleSizeCheckbox->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, 13), "Resizable Window");
	resizableCheckbox->UseCheckIcon(true);
	resizableCheckbox->SetCallback([&](bool checked) { this->ResizableChecked(checked); });
	this->AddComponent(resizableCheckbox);

	descLabel = new Label(resizableCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Allow resizing and maximizing window");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	fullscreenCheckbox = new Checkbox(resizableCheckbox->Below(Point(0, 17)), Point(Checkbox::AUTOSIZE, 13), "Fullscreen");
	fullscreenCheckbox->UseCheckIcon(true);
	fullscreenCheckbox->SetCallback([&](bool checked) { this->FullscreenChecked(checked); });
	this->AddComponent(fullscreenCheckbox);

	descLabel = new Label(fullscreenCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Fill the entire screen");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	altFullscreenCheckbox = new Checkbox(fullscreenCheckbox->Right(Point(70, 0)), Point(Checkbox::AUTOSIZE, 13), "Change Resolution");
	altFullscreenCheckbox->UseCheckIcon(true);
	altFullscreenCheckbox->SetCallback([&](bool checked) { this->AltFullscreenChecked(checked); });
	this->AddComponent(altFullscreenCheckbox);

	descLabel = new Label(altFullscreenCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Old fullscreen");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);


	fastQuitCheckbox = new Checkbox(fullscreenCheckbox->Below(Point(0, 24)), Point(Checkbox::AUTOSIZE, 13), "Fast Quit");
	fastQuitCheckbox->UseCheckIcon(true);
	fastQuitCheckbox->SetCallback([&](bool checked) { this->FastQuitChecked(checked); });
	this->AddComponent(fastQuitCheckbox);

	descLabel = new Label(fastQuitCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Always exit completely when clicking \"X\"");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);

	updatesCheckbox = new Checkbox(fastQuitCheckbox->Below(Point(0, 15)), Point(Checkbox::AUTOSIZE, 13), "Update Check");
	updatesCheckbox->UseCheckIcon(true);
	updatesCheckbox->SetCallback([&](bool checked) { this->UpdatesChecked(checked); });
	this->AddComponent(updatesCheckbox);

	descLabel = new Label(updatesCheckbox->Below(Point(15, 0)), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Check for updates at http://starcatcher.us/TPT");
	descLabel->SetColor(COLRGB(150, 150, 150));
	this->AddComponent(descLabel);


	// TODO: better callback functions for buttons
	class OpenDataFolderAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<OptionsUI*>(button->GetParent()->GetParent())->DataFolderClicked();
		}
	};

	dataFolderButton = new Button(updatesCheckbox->Below(Point(0, 17)), Point(Button::AUTOSIZE, Button::AUTOSIZE), "Open Data Folder");
	dataFolderButton->SetCallback(new OpenDataFolderAction());
	this->AddComponent(dataFolderButton);


	Button *okButton = new Button(Point(0, this->size.Y-15), Point(this->size.X+1, 15), "OK");
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);

	InitializeOptions();
}

void OptionsUI::InitializeOptions()
{
	heatSimCheckbox->SetChecked(!legacy_enable);
	ambientCheckbox->SetChecked(aheat_enable);
	newtonianCheckbox->SetChecked(ngrav_enable);
	waterEqalizationCheckbox->SetChecked(water_equal_test);

	airSimDropdown->SetSelectedOption(airMode);
	gravityDropdown->SetSelectedOption(gravityMode);
	edgeModeDropdown->SetSelectedOption(sim->edgeMode);

	doubleSizeCheckbox->SetChecked(Engine::Ref().GetScale() >= 2);
	resizableCheckbox->SetChecked(Engine::Ref().IsResizable());
	fullscreenCheckbox->SetChecked(Engine::Ref().IsFullscreen());
	altFullscreenCheckbox->SetChecked(Engine::Ref().IsAltFullscreen());

	fastQuitCheckbox->SetChecked(Engine::Ref().IsFastQuit());
	updatesCheckbox->SetChecked(doUpdates);
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
		start_grav_async();
	else
		stop_grav_async();
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
	gravityMode = option;
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

void OptionsUI::DoubleSizeChecked(bool checked)
{
	Engine::Ref().SetScale(checked ? 2 : 1);
}

void OptionsUI::ResizableChecked(bool checked)
{
	Engine::Ref().SetResizable(checked);
}

void OptionsUI::FullscreenChecked(bool checked)
{
	Engine::Ref().SetFullscreen(checked);
}

void OptionsUI::AltFullscreenChecked(bool checked)
{
	Engine::Ref().SetAltFullscreen(checked);
}

void OptionsUI::FastQuitChecked(bool checked)
{
	Engine::Ref().SetFastQuit(checked);
}

void OptionsUI::UpdatesChecked(bool checked)
{
	doUpdates = checked;
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

void OptionsUI::OnDraw(VideoBuffer * buf)
{
	buf->DrawLine(0, edgeModeDropdown->Below(Point(0, 5)).Y, size.X, edgeModeDropdown->Below(Point(0, 5)).Y, 200, 200, 200, 255);
	buf->DrawLine(0, altFullscreenCheckbox->Below(Point(0, 17)).Y, size.X, altFullscreenCheckbox->Below(Point(0, 17)).Y, 200, 200, 200, 255);
}
