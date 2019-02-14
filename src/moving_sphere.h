//
// Created by yu cao on 2019-02-14.
//

#ifndef RAYTRACE_MOVING_SPHERE_H
#define RAYTRACE_MOVING_SPHERE_H


#include "hitable.h"

class moving_sphere : public hitable
{
public:
	moving_sphere() = default;

	moving_sphere(vec3 cen0, vec3 cen1, float t0, float t1, float r, material *m) :
			center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m){}

	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

	vec3 center(float time) const
	{
		return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
	}

private:
	vec3 center0, center1;
	float time0, time1;
	float radius;
	material *mat_ptr;
};

bool moving_sphere::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	vec3 oc = r.origin() - center(r.time());
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;//依旧是交点t的求根公式；变形是因为分子提出了2，所以与分母的2a中的2约去
		if (temp < t_max && temp > t_min)//两个解中t小的那个光线击中了球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)//判定两个解大的那个光线是否击中球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

bool moving_sphere::bounding_box(float t0, float t1, aabb &box) const
{
	aabb box0(center(t0) - vec3(radius, radius, radius), center(t0) + vec3(radius, radius, radius));
	aabb box1(center(t1) - vec3(radius, radius, radius), center(t1) + vec3(radius, radius, radius));
	box = surrounding_box(box0, box1);
	return true;
}

#endif //RAYTRACE_MOVING_SPHERE_H
