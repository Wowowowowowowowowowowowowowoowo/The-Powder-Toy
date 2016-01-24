#include "ErrorPrompt.h"
#include "common/Point.h"
#include "interface/Label.h"
#include "interface/Button.h"

ErrorPrompt::ErrorPrompt(std::string message, std::string dismiss):
	Window_(Point(CENTERED, CENTERED), Point(250, 55))
{
	Label *titleLabel = new Label(Point(5, 3), Point(Label::AUTOSIZE, Label::AUTOSIZE), "Error");
	titleLabel->SetColor(COLRGB(255, 64, 32));
	this->AddComponent(titleLabel);

	Label *messageLabel = new Label(titleLabel->Below(Point(0, 0)), Point(240, Label::AUTOSIZE), message, true);
	this->Resize(Point(CENTERED, CENTERED), Point(250, messageLabel->GetSize().Y+39));
	this->AddComponent(messageLabel);

	Button *okButton = new Button(Point(0, this->size.Y-15), Point(this->size.X, 15), dismiss);
	okButton->SetTextColor(COLRGB(255, 64, 32));
	okButton->SetCloseButton(true);
	this->AddComponent(okButton);
}
