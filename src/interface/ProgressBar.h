#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <string>
#include "Component.h"

namespace gfx
{
class VideoBuffer;
}
class ProgressBar : public Component
{
	int progress = 0;

public:
	ProgressBar(Point position, Point size);

	int GetProgress();
	void SetProgress(int progress);

	void OnDraw(gfx::VideoBuffer* vid) override;
};

#endif // PROGRESSBAR_H
