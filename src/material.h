//
// Created by yu cao on 2019-02-11.
//

#ifndef RAYTRACE_MATERIAL_H
#define RAYTRACE_MATERIAL_H

#include "rays.h"
#include "hitable.h"
#include "texture.h"

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0*vec3(drand48(),drand48(),drand48()) - vec3(1,1,1);
	} while (p.squared_length() >= 1.0);
	return p;
}

//镜面反射的反射光线方向
vec3 reflect(const vec3 &v, const vec3 &n){
	return v - 2 * dot(v, n) * n;
}

//使得折射介质能够随着角度的不同有不同的折射率
float schlick(float cosine, float ref_idx){
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

//折射，如果出现折射，折射的光线通过refracted引用返回
bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);//入射光线方向归一化
	float dt = dot(uv, n);//入射光线与法线作点积，得到结果即为-cosC，C为入射角（点积结果恒为负值）
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);//判定是否发生全反射
	if (discriminant > 0)//未发生全反射
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);//得到折射光线方向
		return true;
	}
	else//全反射
		return false;
}

class material
{
public:
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0;
};

//漫反射
class lambertian : public material
{
public:
	lambertian(texture *a) : albedo(a){}

	//入射光，hit点的的记录，衰减，散射
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();//进行漫反射随机化反射方向
		scattered = ray(rec.p, target - rec.p, r_in.time());//散射光线
		attenuation = albedo->value(0, 0, rec.p);
		return true;
	}

private:
	texture *albedo;//反射率（根据绑定的纹理内容进行处理）
};

class metal : public material{
public:
	metal(const vec3 &a, float f) : albedo(a)
	{
		if (f < 1)
			fuzz = f;
		else
			fuzz = 1;
	}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);//计算出反射光线方向
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);//反射光线与法线呈锐角，证明散射成功
	}

private:
	vec3 albedo;//反射率
	float fuzz;//反射光线模糊率[0,1]
};

//折射介质
class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 outward_normal;//建立一个与入射光线恒为钝角的法线
		vec3 reflected = reflect(r_in.direction(), rec.normal);//计算反射光线方向
		float ni_over_nt;//即为ni/nt（ni是入射光原来的介质，nt是要进入的介质）
		attenuation = vec3(1.0, 1.0, 1.0);//这里表面是glass，即设定折射界面不会吸收光线
		vec3 refracted;//保存计算得到的折射光线的方向

		float reflect_prob;
		float cosine;

		//让outward_normal始终是介质法线，与入射光线呈钝角；确定折光率n之比（ni是入射光原来的介质，nt是要进入的介质）
		if (dot(r_in.direction(), rec.normal) > 0)//从球体内射出（光密射光疏，可能全反射）
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;

			cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = sqrt(1 - ref_idx * ref_idx * (1 - cosine * cosine));
		}
		else//从球体外射入（光密射光密，不可能有全反射）
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;//恒<1，代入下面计算可知一定不可能会全反射

			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			reflect_prob = schlick(cosine, ref_idx);
		else
			reflect_prob = 1.0;
		if (drand48() < reflect_prob)
			scattered = ray(rec.p, reflected);
		else
			scattered = ray(rec.p, refracted);
		return true;
	}

private:
	float ref_idx;//球体材质折光率，一般恒>1
};

#endif //RAYTRACE_MATERIAL_H
