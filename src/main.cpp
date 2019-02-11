#include <iostream>
#include "vec3.h"
#include "rays.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

//depth：进行多少次光线追踪
vec3 color(const ray &r, hitable *world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec))
	{
		ray scattered;//散射光线
		vec3 attenuation;//反射率
		if (depth < 50 && rec.mat_ptr->scatter(r,rec,attenuation,scattered))//调用两个派生类进行分别的渲染
			return attenuation * color(scattered, world, depth + 1);
		else
			return vec3(0,0,0);
	}
	else
	{
		vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
		float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
	}
}

int main()
{
	//画面是200*100
	int nx = 200;
	int ny = 100;
	int ns = 100;//对一个像素点重复采样进行抗锯齿

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//创建两个球体，list[i]这个指针从基类hitable指向派生类sphere
	hitable *list[5];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));//原点(0,0,-1)，半径为0.5，漫反射材质
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2),0.0));//原点(1,0,-1)，半径为0.5，镜面反射材质，模糊度0
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));

	//创建一个list，可以对整个球体的list进行逐一检查
	hitable *world = new hitable_list(list, 5);

	//建立一个照相机
	camera cam;

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++)//通过ns次的模糊化后，进行抗锯齿
			{
				//drand48生成一个[0,1)之间的double
				float u = float(i + drand48()) / float(nx);
				float v = float(j + drand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			col /= float(ns);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}