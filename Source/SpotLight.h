#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H
#include <glm/vec3.hpp>

#include "Shader.h"

class SpotLight
{
public:
	glm::vec3 position;
	glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	SpotLight(const glm::vec3& position, const glm::vec3& direction, float cut_off, float outer_cut_off, float constant,
		float linear, float quadratic, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
		: position(position),
		  direction(direction),
		  cutOff(cut_off),
		  outerCutOff(outer_cut_off),
		  constant(constant),
		  linear(linear),
		  quadratic(quadratic),
		  ambient(ambient),
		  diffuse(diffuse),
		  specular(specular) {}

	// TODO: In future, allow for multiple spotlights
	void SetUniforms(const Shader& shader) const
	{
		shader.setVec3("spotLight.position", position);
		shader.setVec3("spotLight.direction", direction);
		shader.setVec3("spotLight.ambient", ambient);
		shader.setVec3("spotLight.diffuse", diffuse);
		shader.setVec3("spotLight.specular", specular);
		shader.setFloat("spotLight.constant", constant);
		shader.setFloat("spotLight.linear", linear);
		shader.setFloat("spotLight.quadratic", quadratic);
		shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(cutOff)));
		shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));
	}
};

#endif
