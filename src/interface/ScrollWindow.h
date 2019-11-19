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

	bool isMouseInsideScrollbar = false, scrollbarHeld = false;
	int initialClickPos = 0, initialOffset = 0;

public:
	ScrollWindow(Point position, Point size);

	int GetUsableWidth() { return size.X - 8; }
	void SetScrollable(bool scrollable, int maxScroll);
	int GetScrollPosition() { return scrolled; }
	void SetScrollPosition(int pos);
	int GetMaxScrollSize() { return scrollSize; }

protected:
	void OnMouseDown(int x, int y, unsigned char button) override;
	void OnMouseUp(int x, int y, unsigned char button) override;
	void DoMouseWheel(int x, int y, int d) override;
	void OnDraw(gfx::VideoBuffer *buf) override;
	void OnMouseMove(int x, int y, Point difference) override;
};
}
#endif
