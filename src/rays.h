//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_RAYS_H
#define RAYTRACE_RAYS_H

#include "vec3.h"

class ray
{
public:
	ray(){}

	ray(const vec3 &a, const vec3 &b, float ti = 0.0)
	{
		A = a;
		B = b;
		_time = ti;
	}

	vec3 origin() const
	{ return A; }

	vec3 direction() const
	{ return B; }

	float time() const
	{ return _time; }

	vec3 point_at_parameter(float t) const
	{ return A + t * B; }

private:
	vec3 A;//光源
	vec3 B;//朝向
	float _time;//光线射出的时间（相比较快门按下之后）
};

#endif //RAYTRACE_RAYS_H
