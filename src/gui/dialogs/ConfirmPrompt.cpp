#include "ConfirmPrompt.h"
#include "common/Point.h"
#include "interface/Label.h"
#include "interface/Button.h"

ConfirmPrompt::ConfirmPrompt(ConfirmAction *confirmAction, std::string title, std::string message, std::string OK, std::string cancel):
	Window_(Point(CENTERED, CENTERED), Point(250, 55)),
	confirmAction(confirmAction),
	wasConfirmed(false)
{
	Label *titleLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), title);
	titleLabel->SetColor(COLRGB(140, 140, 255));
	this->AddComponent(titleLabel);

	Label *messageLabel = new Label(titleLabel->Below(Point(0, 0)), Point(240, Label::AUTOSIZE), message, true);
	this->Resize(Point(CENTERED, CENTERED), Point(250, messageLabel->GetSize().Y+39));
	this->AddComponent(messageLabel);

	class ConfirmButtonAction : public ButtonAction
	{
	public:
		virtual void ButtionActionCallback(Button *button, unsigned char b)
		{
			dynamic_cast<ConfirmPrompt*>(button->GetParent())->wasConfirmed = true;
		}
	};
	Button *cancelButton = new Button(Point(0, this->size.Y-15), Point(2*this->size.X/3+1, 15), cancel);
	cancelButton->SetCloseButton(true);
	this->AddComponent(cancelButton);

	Button *okButton = new Button(Point(2*this->size.X/3, this->size.Y-15), Point(this->size.X/3+1, 15), OK);
	okButton->SetCallback(new ConfirmButtonAction());
	okButton->SetTextColor(COLRGB(140, 140, 255));
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);
}

void ConfirmPrompt::OnKeyPress(int key, unsigned short character, unsigned short modifiers)
{
	if (key == SDLK_RETURN)
	{
		this->toDelete = true;
	}
}

ConfirmPrompt::~ConfirmPrompt()
{
	confirmAction->Action(wasConfirmed);
}
