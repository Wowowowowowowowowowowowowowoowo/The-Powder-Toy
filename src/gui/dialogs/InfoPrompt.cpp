#include "InfoPrompt.h"
#include "common/Point.h"
#include "interface/Label.h"
#include "interface/Button.h"

InfoPrompt::InfoPrompt(std::string title, std::string message, std::string OK):
	Window_(Point(CENTERED, CENTERED), Point(250, 55))
{
	Label *titleLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), title);
	titleLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(titleLabel);

	Label *messageLabel = new Label(titleLabel->Below(Point(0, 0)), Point(240, Label::AUTOSIZE), message, true);
	this->Resize(Point(CENTERED, CENTERED), Point(250, messageLabel->GetSize().Y+39));
	this->AddComponent(messageLabel);

	Button *okButton = new Button(Point(0, this->size.Y-15), Point(this->size.X, 15), OK);
	okButton->SetTextColor(COLRGB(140, 140, 255));
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);
}

void InfoPrompt::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (key == SDLK_RETURN)
		this->toDelete = true;
}
