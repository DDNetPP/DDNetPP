/* ddnet++ math */
#ifndef BASE_MATH_DDPP_H
#define BASE_MATH_DDPP_H

#include "vmath.h"

#include <math.h>
#include <stdlib.h>

// M_PI defined in math.h on linux bot not on windows
#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

inline int _random(int max) { return rand() % max; }
inline int _random(int min, int max) { return min + rand() % (max + 1 - min); }

template<typename T>
inline vector2_base<T> rotate_around_point(vector2_base<T> point, vector2_base<T> pivot, float angle)
{
	T s = sin(angle);
	T c = cos(angle);
	// translate point to origin
	point -= pivot;
	// rotate point
	vector2_base<T> rotated_point;
	rotated_point.x = point.x * c - point.y * s;
	rotated_point.y = point.x * s + point.y * c;
	// translate point back to pivot
	point = rotated_point + pivot;
	return point;
}

#endif
