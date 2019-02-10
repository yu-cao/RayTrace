//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_HITABLE_LIST_H
#define RAYTRACE_HITABLE_LIST_H
#include "hitable.h"

class hitable_list: public hitable  {
public:
	hitable_list() {}
	hitable_list(hitable **l, int n) {list = l; list_size = n; }
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	hitable **list;
	int list_size;
};

bool hitable_list::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for (int i = 0; i < list_size; i++)
	{
		//如果成功，temp_rec将会被刷新成包含击中的t值，击中点的光线矢量，击中点的表面法线组成的rec
		if (list[i]->hit(r, t_min, closest_so_far, temp_rec))
		{
			hit_anything = true;//确认这里是被光线击中，要被渲染的

			//刷新成返回点的t值，供下一个球体作为t_max值使用
			//不然后续的球体渲染得到的t会直接覆盖前面球体的t结果，导致前后渲染顺序出错
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

#endif //RAYTRACE_HITABLE_LIST_H
