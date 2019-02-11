//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_SPHERE_H
#define RAYTRACE_SPHERE_H

#include "hitable.h"

class sphere : public hitable
{
public:
	sphere(){}
	sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m){}

	virtual bool hit(const ray &r, float tmin, float tmax, hit_record &rec) const;

private:
	vec3 center;
	float radius;
	material *mat_ptr;
};

bool sphere::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());//注意原来这里是b = 2 * dot(oc, r.direction()); 这个2提出来和4ac一起拿到根号外了
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;//这里不是b^2 - 4ac，原因见上
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;//依旧是交点t的求根公式；变形是因为分子提出了2，所以与分母的2a中的2约去
		if (temp < t_max && temp > t_min)//两个解中t小的那个光线击中了球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)//判定两个解大的那个光线是否击中球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

#endif //RAYTRACE_SPHERE_H
