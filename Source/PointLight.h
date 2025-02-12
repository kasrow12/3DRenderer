#pragma once

#include <glm/vec3.hpp>

class PointLight
{
public:
	glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	PointLight(const glm::vec3& position, float constant, float linear, float quadratic, const glm::vec3& ambient,
		const glm::vec3& diffuse, const glm::vec3& specular)
		: position(position),
		  constant(constant),
		  linear(linear),
		  quadratic(quadratic),
		  ambient(ambient),
		  diffuse(diffuse),
		  specular(specular) {}

	void SetUniforms(const Shader& shader, size_t i) const
	{
		shader.setVec3("pointLights[" + std::to_string(i) + "].position", position);
		shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", ambient);
		shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", diffuse);
		shader.setVec3("pointLights[" + std::to_string(i) + "].specular", specular);
		shader.setFloat("pointLights[" + std::to_string(i) + "].constant", constant);
		shader.setFloat("pointLights[" + std::to_string(i) + "].linear", linear);
		shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", quadratic);
	}
};
