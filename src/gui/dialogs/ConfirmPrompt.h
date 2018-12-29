#ifndef COMFIRMPROMPT_H
#define COMFIRMPROMPT_H
#include <string>
#include "interface/Window.h"

class ConfirmAction
{
public:
	virtual void Action(bool isConfirmed) = 0;
	virtual ~ConfirmAction() {}
};

class ConfirmPrompt : public Window_
{
	ConfirmAction *confirmAction;
public:
	ConfirmPrompt(ConfirmAction *confirmAction, std::string title, std::string message, std::string OK = "OK", std::string cancel = "Cancel");
	~ConfirmPrompt();
	bool wasConfirmed;
	void Action(bool isConfirmed);

	virtual void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
};

#endif
