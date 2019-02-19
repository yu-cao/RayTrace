//
// Created by yu cao on 2019-02-19.
//

#ifndef RAYTRACE_AA_RECT_H
#define RAYTRACE_AA_RECT_H

#include "hitable.h"

class xy_rect : public hitable
{
public:
	xy_rect(){}
	xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, material *mat) : x0(_x0), x1(_x1), y0(_y0), y1(_y1),
																				   k(_k), mp(mat){};

	virtual bool hit(const ray &r, float t0, float t1, hit_record &rec) const;

	virtual bool bounding_box(float t0, float t1, aabb &box) const
	{
		box = aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));//aabb盒取矩阵的左下角和右上角，包含一个薄平面
		return true;
	}

	material *mp;
	float x0, x1, y0, y1, k;
};

class xz_rect : public hitable
{
public:
	xz_rect(){}
	xz_rect(float _x0, float _x1, float _z0, float _z1, float _k, material *mat) : x0(_x0), x1(_x1), z0(_z0), z1(_z1),
																				   k(_k), mp(mat){}

	virtual bool hit(const ray &r, float t0, float t1, hit_record &rec) const;

	virtual bool bounding_box(float t0, float t1, aabb &box) const
	{
		box = aabb(vec3(x0, k - 0.0001, z0), vec3(x1, k + 0.0001, z1));
		return true;
	}

	material *mp;
	float x0, x1, z0, z1, k;
};

class yz_rect : public hitable
{
public:
	yz_rect(){}
	yz_rect(float _y0, float _y1, float _z0, float _z1, float _k, material *mat) : y0(_y0), y1(_y1), z0(_z0), z1(_z1),
																				   k(_k), mp(mat){};

	virtual bool hit(const ray &r, float t0, float t1, hit_record &rec) const;

	virtual bool bounding_box(float t0, float t1, aabb &box) const
	{
		box = aabb(vec3(k - 0.0001, y0, z0), vec3(k + 0.0001, y1, z1));
		return true;
	}

	material *mp;
	float y0, y1, z0, z1, k;
};

bool xy_rect::hit(const ray &r, float t0, float t1, hit_record &rec) const
{
	float t = (k - r.origin().z()) / r.direction().z();//通过k计算得到t值，并且判断合理性
	if (t < t0 || t > t1)
		return false;
	float x = r.origin().x() + t * r.direction().x();//计算得到x和y并判断合理性
	float y = r.origin().y() + t * r.direction().y();
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;
	rec.u = (x - x0) / (x1 - x0);//得到光线击中点在矩形表面的uv值
	rec.v = (y - y0) / (y1 - y0);
	rec.t = t;
	rec.mat_ptr = mp;//材质绑定
	rec.p = r.point_at_parameter(t);//击中点的光线常数：(A+tB)的值
	rec.normal = vec3(0, 0, 1);//因为是xy平面，必定与z轴垂直，所以z = 1即是法线方向
	return true;
}

bool xz_rect::hit(const ray &r, float t0, float t1, hit_record &rec) const
{
	float t = (k - r.origin().y()) / r.direction().y();
	if (t < t0 || t > t1)
		return false;
	float x = r.origin().x() + t * r.direction().x();
	float z = r.origin().z() + t * r.direction().z();
	if (x < x0 || x > x1 || z < z0 || z > z1)
		return false;
	rec.u = (x - x0) / (x1 - x0);
	rec.v = (z - z0) / (z1 - z0);
	rec.t = t;
	rec.mat_ptr = mp;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 1, 0);
	return true;
}

bool yz_rect::hit(const ray &r, float t0, float t1, hit_record &rec) const
{
	float t = (k - r.origin().x()) / r.direction().x();
	if (t < t0 || t > t1)
		return false;
	float y = r.origin().y() + t * r.direction().y();
	float z = r.origin().z() + t * r.direction().z();
	if (y < y0 || y > y1 || z < z0 || z > z1)
		return false;
	rec.u = (y - y0) / (y1 - y0);
	rec.v = (z - z0) / (z1 - z0);
	rec.t = t;
	rec.mat_ptr = mp;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(1, 0, 0);
	return true;
}

#endif //RAYTRACE_AA_RECT_H
