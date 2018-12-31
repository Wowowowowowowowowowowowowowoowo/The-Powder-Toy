#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <functional>
#include <string>
#include <vector>
#include "Component.h"
#include "common/Point.h"
#include "interface/Window.h"

class VideoBuffer;

class Dropdown : public Component
{
	std::vector<std::string> options;
	unsigned int selectedOption = 0;
	bool isSelectingOption = false;
	//DropdownAction *callback = nullptr;
	std::function<void(unsigned int)> callback = nullptr;

public:
	Dropdown(Point position, Point size, std::vector<std::string> options);

	void SetCallback(std::function<void(unsigned int)> callback) { this->callback = callback; }
	unsigned int GetSelectedOption() { return selectedOption; }
	void SetSelectedOption(unsigned int selectedOption);
	bool IsSelectingOption() { return isSelectingOption; }

	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnDraw(VideoBuffer* vid);

	static const int AUTOSIZE = -1;

	friend class DropdownOptions;
};

class DropdownOptions : public Window_
{
	Dropdown * dropdown;
	unsigned int hoveredOption = -1;

public:
	DropdownOptions(Point position, Point size, Dropdown * dropdown);
	~DropdownOptions();

	virtual void OnMouseMove(int x, int y, Point difference);
	virtual void OnMouseUp(int x, int y, unsigned char button);
	virtual void OnDraw(VideoBuffer* vid);
};

#endif
