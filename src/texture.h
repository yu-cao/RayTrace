//
// Created by yu cao on 2019-02-15.
//

#ifndef RAYTRACE_TEXTURE_H
#define RAYTRACE_TEXTURE_H

#include "perlin.h"

class texture  {
public:
	//表示某个uv点的rgb值的接口
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class constant_texture : public texture {
public:
	constant_texture() { }
	constant_texture(vec3 c) : color(c) { }

	virtual vec3 value(float u, float v, const vec3& p) const {
		return color;
	}

private:
	vec3 color;
};

//做一个国际象棋棋盘
class checker_texture : public texture{
public:
	checker_texture(){}
	checker_texture(texture *t0, texture *t1) : even(t0), odd(t1){}

	virtual vec3 value(float u, float v, const vec3 &p) const
	{
		float sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

private:
	texture *odd;
	texture *even;
};

class noise_texture : public texture {
public:
	noise_texture() {}
	noise_texture(float scale): scale(scale){}

	virtual vec3 value(float u, float v, const vec3& p) const {
		return vec3(1,1,1)*noise.noise(scale * p);
	}

private:
	perlin noise;
	float scale;
};


#endif //RAYTRACE_TEXTURE_H
