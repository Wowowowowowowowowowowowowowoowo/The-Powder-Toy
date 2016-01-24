#ifndef INFOPROMPT_H
#define INFOPROMPT_H
#include <string>
#include "interface/Window.h"

class InfoPrompt : public Window_
{
public:
	InfoPrompt(std::string title, std::string message, std::string OK = "OK");
};

#endif
