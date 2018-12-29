#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Label.h"

class VideoBuffer;
class Textbox;
class TextboxAction
{
public:
	TextboxAction() { }
	virtual ~TextboxAction() { }
	virtual void TextChangedCallback(Textbox *textbox) { }
};

class Textbox : public Label
{
public:
	static const int NOSIZELIMIT = 0; // Component size, not text size
	enum texttype { TEXT, MULTILINE, NUMBER };

	Textbox(Point position, Point size, std::string text, bool multiline = false);
	~Textbox();

	void SetCallback(TextboxAction *callback_);

	virtual void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	virtual void OnTextInput(const char *text);
	virtual void OnDraw(VideoBuffer* vid);
	virtual void OnFocus();

	void SetAutoSize(bool X, bool Y, Point limit);
	void SetCharacterLimit(int characterLimit_) { characterLimit = characterLimit_; }
	void SetAutoCorrect(bool autoCorrect_) { autoCorrect = autoCorrect_; }
	void SetType(texttype type_) { type = type_; }

private:
	Point sizeLimit;
	unsigned int characterLimit;
	bool autoCorrect;
	TextboxAction *callback;
	texttype type;

	bool DeleteHighlight(bool updateDisplayText);
	void InsertText(std::string inserttext);

	bool CharacterValid(char c);
	bool StringValid(const char *str);

protected:
	bool ShowCursor() { return true; }
};

#endif
