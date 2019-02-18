//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_HITABLE_H
#define RAYTRACE_HITABLE_H

#include "aabb.h"
#include "math.h"

class material;

//通过坐标变换得到球面u，v
void get_sphere_uv(const vec3 &p, float &u, float &v)
{
	float phi = atan2(p.z(), p.x());
	float theta = asin(p.y());
	u = 1 - (phi + M_PI) / (2 * M_PI);
	v = (theta + M_PI / 2) / M_PI;
}

struct hit_record
{
	float t;//击中时的t值
	float u,v;//球面对应的u,v值
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
