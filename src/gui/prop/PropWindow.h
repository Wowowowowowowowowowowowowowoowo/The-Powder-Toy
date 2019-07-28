#ifndef PROPWINDOW_H
#define PROPWINDOW_H
#include "common/Point.h"
#include "interface/Window.h"
#include "simulation/StructProperty.h"

class Dropdown;
class PropTool;
class Textbox;
class PropWindow : public ui::Window
{

	Dropdown *propertyDropdown;
	Textbox *valueTextbox;

	PropTool *propTool;

	std::vector<StructProperty> properties;
	unsigned int selectedProperty = 0;

	void UpdatePropTool();

	template<typename T>
	T ParseNumber(const std::string& num, bool isHex, bool &isParsed);
	bool ParseInteger(const std::string& num, bool isHex);
	bool ParseValue(std::string value);
public:
	PropWindow();

	void OnPropertyChanged(unsigned int option);

	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
};

#endif // PROPWINDOW_H
