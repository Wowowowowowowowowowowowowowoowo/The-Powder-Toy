#ifndef ERRORPROMPT_H
#define ERRORPROMPT_H
#include <string>
#include "interface/Window.h"

class ErrorPrompt : public ui::Window
{
public:
	ErrorPrompt(std::string message, std::string dismiss = "Dismiss");
};

#endif
