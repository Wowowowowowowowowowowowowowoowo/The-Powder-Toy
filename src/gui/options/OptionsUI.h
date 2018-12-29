#ifndef OPTIONS_H
#define OPTIONS_H
#include "interface/Window.h"

class Button;
class Checkbox;
class Dropdown;
class Simulation;
class OptionsUI : public Window_
{
	Checkbox *heatSimCheckbox, *ambientCheckbox, *newtonianCheckbox, *waterEqalizationCheckbox;
	Dropdown *airSimDropdown, *gravityDropdown, *edgeModeDropdown;

	Checkbox *doubleSizeCheckbox, *resizableCheckbox, *fullscreenCheckbox, *altFullscreenCheckbox;
	Checkbox *fastQuitCheckbox, *updatesCheckbox;
	Button *dataFolderButton;

	Simulation * sim;

	void InitializeOptions();
	void HeatSimChecked(bool checked);
	void AmbientChecked(bool checked);
	void NewtonianChecked(bool checked);
	void WaterEqualizationChecked(bool checked);
	void AirSimSelected(unsigned int option);
	void GravitySelected(unsigned int option);
	unsigned int oldEdgeMode;
	void EdgeModeSelected(unsigned int option);
	void DoubleSizeChecked(bool checked);
	void ResizableChecked(bool checked);
	void FullscreenChecked(bool checked);
	void AltFullscreenChecked(bool checked);
	void FastQuitChecked(bool checked);
	void UpdatesChecked(bool checked);
	void DataFolderClicked();

	virtual void OnDraw(VideoBuffer * buf);

public:
	OptionsUI(Simulation * sim);
};

#endif
