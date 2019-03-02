#ifndef SCROLLWINDOW_H
#define SCROLLWINDOW_H

#include "Window.h"

namespace ui
{
class ScrollWindow : public Window
{
	bool scrollable;
	int scrollSize, scrolled;
	int lastMouseX, lastMouseY;

public:
	ScrollWindow(Point position, Point size);

	void SetScrollable(bool scroll, int maxScroll);
	int GetScrollPosition() { return scrolled; }
	void SetScrollPosition(int pos);
	int GetMaxScrollSize() { return scrollSize; }

protected:
	void DoMouseWheel(int x, int y, int d) override;
};
}
#endif
