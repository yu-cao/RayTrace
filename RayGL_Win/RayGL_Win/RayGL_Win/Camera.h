#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Camera
{
public:
	Camera(glm::vec3 lookFrom, glm::vec3 lookAt, glm::vec3 up, float vfov, float aspect, float aperture = 0)
	{
		float theta = glm::radians(vfov);
		float half_height = tan(theta * 0.5f);
		float half_width = aspect * half_height;

		lens_radius = aperture * 0.5f;

		origin = lookFrom;

		glm::vec3 w = glm::normalize(lookFrom - lookAt);
		glm::vec3 u = glm::normalize(glm::cross(up, w));
		glm::vec3 v = glm::cross(w, u);

		lower_left_corner = origin - (half_width * u + half_height * v + w);
		horizontal = 2.0f * half_width * u;
		vertical = 2.0f * half_height* v;
	}

	void Bind(Shader<ShaderType::COMPUTE>& shaderCompute)
	{
		shaderCompute.use();
		shaderCompute.setVector("cam.origin", origin);
		shaderCompute.setVector("cam.lower_left", lower_left_corner);
		shaderCompute.setVector("cam.horz", horizontal);
		shaderCompute.setVector("cam.vert", vertical);
		shaderCompute.setFloat("cam.lens_radius", lens_radius);
	}
private:
	glm::vec3 origin;//光线原点
	glm::vec3 lower_left_corner;//画面左下角
	glm::vec3 horizontal;//画面水平矢量
	glm::vec3 vertical;//画面垂直矢量
	float lens_radius;
};