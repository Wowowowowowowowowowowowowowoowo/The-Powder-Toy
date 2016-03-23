#ifndef TPT_MATH_H
#define TPT_MATH_H

#include <cmath>

static int remainder_p(int x, int y)
{
	return (x % y) + (x>=0 ? 0 : y);
}

static float remainder_p(float x, float y)
{
	return std::fmod(x, y) + (x>=0 ? 0 : y);
}

#endif
