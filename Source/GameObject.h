#pragma once

#include "Model.h"
#include "Transform.h"

class GameObject
{
public:
    Model* model;
    Transform transform;
    std::string name;

    // Movement parameters for train
    bool isMoving = false;
    // Radius of circular path
    float radius = 15.0f;
    float speed = 0.5f;
    float currentAngle = 0.0f;

    GameObject(Model* model, const Transform& transform, const std::string& name)
        : model(model), transform(transform), name(name) {}

    void update(float deltaTime)
    {
        if (isMoving)
        {
            // Update position in a circle
            currentAngle += speed * deltaTime;
            transform.position.x = radius * sin(currentAngle);
            transform.position.z = radius * cos(currentAngle);

            // Make the train face the direction of movement
            transform.rotation.y = glm::degrees(atan2(-sin(currentAngle), -cos(currentAngle)));
        }
    }

    void draw(Shader& shader) const
    {
        shader.setMat4("model", transform.getModelMatrix());
        model->Draw(shader);
    }
};
