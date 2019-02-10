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

	ray(const vec3 &a, const vec3 &b)
	{
		A = a;
		B = b;
	}

	vec3 origin() const
	{ return A; }

	vec3 direction() const
	{ return B; }

	vec3 point_at_parameter(float t) const
	{ return A + t * B; }

	vec3 A;//光源
	vec3 B;//朝向
};

#endif //RAYTRACE_RAYS_H
