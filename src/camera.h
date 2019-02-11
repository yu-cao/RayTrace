//
// Created by yu cao on 2019-02-10.
//

#ifndef RAYTRACE_CAMERA_H
#define RAYTRACE_CAMERA_H
#include "rays.h"

class camera{
public:
	camera()
	{
		lower_left_corner = vec3(-2.0f, -1.0f, -1.0f);//画面左下角
		horizontal = vec3(4.0f, 0.0f, 0.0f);//画面水平长度
		vertical = vec3(0.0f, 2.0f, 0.0f);//画面垂直长度
		origin = vec3(0.0f, 0.0f, 0.0f);//光线原点
	}

	ray get_ray(float u, float v)// 光线从光源点射向z = -1的平面的每一个点
	{
		return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
	}

private:
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
};

#endif //RAYTRACE_CAMERA_H
