//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_CAMERA_H
#define RAYTRACE_CAMERA_H
#include "rays.h"

vec3 random_in_unit_disk(){
	vec3 p;
	do
	{
		p = 2.0 * vec3(drand48(), drand48(), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

class camera{
public:
	//vfov：视野域，也就是视野的角度；aspect：屏幕width与height之比；aperture：光圈（透镜的直径）；focus_dist：焦距
	camera(vec3 lookfrom, vec3 lookat, const vec3 vup, const float vfov, const float aspect,
		   float aperture, float focus_dist, float t0, float t1)
	{
		time0 = t0;
		time1 = t1;
		lens_radius = aperture / 2;

		float theta = vfov * M_PI / 180.0;//弧度制
		float half_height = tan(theta / 2.0);//观察视野一半的高度
		float half_width = aspect * half_height;//通过屏幕的长宽比算出宽度

		origin = lookfrom;

		//建立以相机坐标为原点的一组正交基，现在以相机是坐标原点（投影变换）
		w = unit_vector(lookfrom - lookat);//从被观察点指向相机
		u = unit_vector(cross(vup, w));//vup为x轴，w为y轴进行右手定则得到u的方向（也就是相机画面里向右延展的方向）
		v = cross(w, u);//以w为x轴，u为y轴进行右手定则得到v的方向（两个单位向量叉乘还是单位向量）（也就是相机画面里向上延展的方向）

		lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
		horizontal = 2 * half_width * focus_dist * u;//计算width的全长矢量（要额外加入焦距变量）
		vertical = 2 * half_height * focus_dist * v;
	}

	ray get_ray(float s, float t)
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();
		float time = time0 + drand48() * (time1 - time0);//使得随机在[time0,time1)的时间段内产生一条光线
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, time);
	}

private:
	vec3 origin;//光线原点
	vec3 lower_left_corner;//画面左下角
	vec3 horizontal;//画面水平矢量
	vec3 vertical;//画面垂直矢量
	vec3 u, v, w;//一组相机空间的正交基
	float time0, time1;

	float lens_radius;
};

#endif //RAYTRACE_CAMERA_H
