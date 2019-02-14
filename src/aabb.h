//
// Created by yu cao on 2019-02-14.
//

#ifndef RAYTRACE_AABB_H
#define RAYTRACE_AABB_H
#include "rays.h"
#include "hitable.h"

inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class aabb {
public:
	aabb() {}
	aabb(const vec3& a, const vec3& b) { _min = a; _max = b;}

	vec3 min() const {return _min; }
	vec3 max() const {return _max; }

	bool hit(const ray& r, float tmin, float tmax) const {
		for (int a = 0; a < 3; a++) //分别针对xyz三个方向进行判定，找到光线与box相交的两个t
		{
			float t0 = ffmin((_min[a] - r.origin()[a]) / r.direction()[a],
							 (_max[a] - r.origin()[a]) / r.direction()[a]);
			float t1 = ffmax((_min[a] - r.origin()[a]) / r.direction()[a],
							 (_max[a] - r.origin()[a]) / r.direction()[a]);
			tmin = ffmax(t0, tmin);
			tmax = ffmin(t1, tmax);
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

private:
	//可以想象成一个是盒子左下角，一个是盒子右上角
	vec3 _min;//最小顶点
	vec3 _max;//最大顶点
};

//把两个box拼起来组成一个大的box
aabb surrounding_box(aabb box0, aabb box1) {
	vec3 small( fmin(box0.min().x(), box1.min().x()),
				fmin(box0.min().y(), box1.min().y()),
				fmin(box0.min().z(), box1.min().z()));
	vec3 big  ( fmax(box0.max().x(), box1.max().x()),
				fmax(box0.max().y(), box1.max().y()),
				fmax(box0.max().z(), box1.max().z()));
	return aabb(small,big);
}

#endif //RAYTRACE_AABB_H
