#include "stdafx.h"

#include "MathHelper.h"

const float MathHelper::PI = 3.1415196535f;

float MathHelper::AngleFromXY(float x, float y)
{
	float Theta = 0.0f;

	if (x >= 0.0f)
	{
		Theta = atanf(y / x);
		if (Theta < 0.0f)
		{
			Theta += 2.0f* PI;
		}
	}
	else
	{
		Theta = atanf(y / x) + PI;
	}

	return Theta;
}

