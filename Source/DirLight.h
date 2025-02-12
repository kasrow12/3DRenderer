#pragma once

#include <glm/vec3.hpp>

#include "Shader.h"

class DirLight
{
public:
	glm::vec3 direction;

	glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

	DirLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
		: direction(direction),
		ambient(ambient),
		diffuse(diffuse),
		specular(specular) {}

	void SetUniforms(const Shader& shader) const
	{
		shader.setVec3("dirLight.direction", direction);
		shader.setVec3("dirLight.ambient", ambient);
		shader.setVec3("dirLight.diffuse", diffuse);
		shader.setVec3("dirLight.specular", specular);
	}
};
