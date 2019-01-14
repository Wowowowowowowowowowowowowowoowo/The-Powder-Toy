#include "ConfirmPrompt.h"
#include "common/Point.h"
#include "interface/Label.h"
#include "interface/Button.h"

ConfirmPrompt::ConfirmPrompt(std::function<void(bool)> confirmAction, std::string title, std::string message, std::string OK, std::string cancel):
	Window_(Point(CENTERED, CENTERED), Point(250, 55)),
	confirmAction(confirmAction),
	wasConfirmed(false)
{
#ifndef TOUCHUI
	int buttonHeight = 15;
#else
	int buttonHeight = 25;
#endif

	Label *titleLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), title);
	titleLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(titleLabel);

	Label *messageLabel = new Label(titleLabel->Below(Point(0, 0)), Point(240, Label::AUTOSIZE), message, true);
	this->Resize(Point(CENTERED, CENTERED), Point(250, messageLabel->GetSize().Y + 24 + buttonHeight));
	this->AddComponent(messageLabel);

	Button *cancelButton = new Button(Point(0, this->size.Y - buttonHeight), Point(2 * this->size.X / 3 + 1, buttonHeight), cancel);
	cancelButton->SetCloseButton(true);
	this->AddComponent(cancelButton);

	Button *okButton = new Button(Point(2 * this->size.X / 3, this->size.Y - buttonHeight), Point(this->size.X / 3 + 1, buttonHeight), OK);
	okButton->SetCallback([&](int mb) { this->wasConfirmed = true; });
	okButton->SetTextColor(COLRGB(140, 140, 255));
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);
}

void ConfirmPrompt::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_RETURN)
	{
		this->wasConfirmed = true;
		this->toDelete = true;
	}
}

ConfirmPrompt::~ConfirmPrompt()
{
	confirmAction(wasConfirmed);
}
