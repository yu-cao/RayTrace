//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_HITABLE_H
#define RAYTRACE_HITABLE_H

#include "aabb.h"

class material;

struct hit_record
{
	float t;//击中时的t值
	vec3 p;//击中点的光线
	vec3 normal;//击中点的表面法线（归一化后）
	material *mat_ptr;
};

class hitable
{
public:
	virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const = 0;
	virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};

#endif //RAYTRACE_HITABLE_H
