#include "directX.h"
#include "easing.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float easeOutElastic(float x) {
	const float c4 = (2.0f * (float)M_PI) / 3.0f;

	if (x == 0.0f) {
		return 0.0f;
	}
	else if (x == 1.0f) {
		return 1.0f;
	}
	else {
		return powf(2.0f, -10.0f * x) * sinf((x * 10.0f - 0.75f) * c4) + 1.0f;
	}
}

float easeInOutCubic(float x)
{
	if (x < 0.5f)
	{
		return 4.0f * x * x * x;
	}
	else
	{
		return 1.0f - powf(-2.0f * x + 2.0f, 3.0f) / 2.0f;
	}
}
