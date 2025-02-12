#pragma once
#include <vector>

#include "Camera.h"
#include "DirLight.h"
#include "GameObject.h"
#include "Model.h"
#include "PointLight.h"
#include "SpotLight.h"

class Scene
{
public:
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<PointLight> pointLights;
    SpotLight spotLight;
    DirLight dirLight;
    Model* sphereModel; // for point lights
    float fogDistance = 60.0f;
    bool isDayLight = true;
    
	bool useBlinn = true;
    float tessLevel = 32.0f;

    int screenWidth = 1400;
    int screenHeight = 900;

    Camera camera = Camera({ 0.0f, 1.0f, 15.0f });
    glm::vec3 skyColor = { 0.2f, 0.3f, 0.3f };

    glm::vec3 trainLightDirection = { -1.0f, -0.25f, 0.0f };

    // Bezier
    std::vector<glm::vec3> controlPoints = {
        // Bottom row
        glm::vec3(-1.0f, 0.5f, -1.0f),
        glm::vec3(-0.33f, 0.0f, -1.0f),
        glm::vec3(0.33f, 0.0f, -1.0f),
        glm::vec3(1.0f, 0.0f, -1.0f),
        // Second row
        glm::vec3(-1.0f, 0.0f, -0.33f),
        glm::vec3(-0.33f, 0.0f, -0.33f),
        glm::vec3(0.33f, 0.0f, -0.33f),
        glm::vec3(1.0f, 0.2f, -0.33f),
        // Third row
        glm::vec3(-1.0f, 0.0f, 0.33f),
        glm::vec3(-0.33f, 0.0f, 0.33f),
        glm::vec3(0.33f, 0.0f, 0.33f),
        glm::vec3(1.0f, 0.0f, 0.33f),
        // Top row
        glm::vec3(-1.0f, -0.3f, 1.0f),
        glm::vec3(-0.33f, 0.0f, 1.0f),
        glm::vec3(0.33f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.5f, 1.0f)
    };
    Transform bezierTransform;
    float time = 0.0f;
    float animationSpeed = 3.5f;
    float animationAmplitude = 0.005f;

    Scene() :
        spotLight(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), 12.5f, 15.0f, 1.0f, 0.008f, 0.001f,
            glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f)),
        dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f))
    {
        generateLights();
        bezierTransform.position = glm::vec3(3.0f, 1.0f, 10.0f);
        bezierTransform.scale = glm::vec3(3);
    }

    void update(float deltaTime)
    {
        if (gameObjects.empty())
            return;

        gameObjects[0]->update(deltaTime);
        updateSpotlight();
        updateControlPoints(deltaTime);
    }

    void updateNight()
    {
        if (isDayLight)
        {
            skyColor = glm::vec3(0.2f, 0.3f, 0.3f);
            dirLight.ambient = glm::vec3(0.05f);
            dirLight.diffuse = glm::vec3(0.4f);
            dirLight.specular = glm::vec3(0.5f);
        }
        else
        {
            skyColor = glm::vec3(0.0f, 0.0f, 0.0f);
            dirLight.ambient = glm::vec3(0.05f);
            dirLight.diffuse = glm::vec3(0.0f);
            dirLight.specular = glm::vec3(0.0f);
        }
    }

    void draw(Shader& shader, Shader& lightShader, Shader& tessellationShader)
    {
        glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        setupShaderUniforms(shader);
        drawObjects(shader);
        setupLightUniforms(lightShader);
        drawLights(lightShader);
        drawTessellated(tessellationShader);
    }

private:
    void generateLights()
    {
        pointLights.clear();
        pointLights.reserve(4);

        const glm::vec3 ambient(0.05f);

        std::vector<glm::vec3> positions = {
            glm::vec3(0.7f, 3.2f, 10.0f),
            glm::vec3(2.3f, 3.3f, -4.0f),
            glm::vec3(-4.0f, 2.0f, -12.0f),
            glm::vec3(0.0f, 0.7f, -3.0f)
        };

        std::vector<glm::vec3> colors = {
            glm::vec3(0.1f, 0.8f, 0.2f),
            glm::vec3(0.1f, 0.2f, 0.7f),
            glm::vec3(0.8f, 0.1f, 0.1f),
            glm::vec3(0.8f, 0.8f, 0.8f)
        };

        // Create 4 point lights (4 is also the number of point lights in the shader)
        for (int i = 0; i < 4; i++)
            pointLights.emplace_back(positions[i], 1.0f, 0.09f, 0.002f, ambient, colors[i], colors[i]);
    }

    void setupShaderUniforms(Shader& shader)
    {
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setBool("blinn", useBlinn);
        shader.setFloat("material.shininess", 32.0f); // no specular map

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", camera.getViewMatrix());

        // Lights
        dirLight.SetUniforms(shader);
        for (size_t i = 0; i < pointLights.size(); i++)
        {
            pointLights[i].SetUniforms(shader, i);
        }
        spotLight.SetUniforms(shader);

        // Fog
        shader.setFloat("fogDistance", fogDistance);
        shader.setVec3("skyColor", skyColor);
    }

    void setupLightUniforms(Shader& lightShader)
    {
        lightShader.use();
        lightShader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom),
            (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f));
        lightShader.setMat4("view", camera.getViewMatrix());
        lightShader.setVec3("viewPos", camera.Position);
        lightShader.setFloat("fogDistance", fogDistance);
        lightShader.setVec3("skyColor", skyColor);
    }

    // Spotlight fixed to first object (train)
    void updateSpotlight()
    {
        static glm::vec3 localOffset(-9, 3.5f, 0.0f);

        if (gameObjects.empty())
            return;

        const auto& transform = gameObjects[0]->transform;
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        spotLight.position = transform.position + glm::vec3(rotationMatrix * glm::vec4(localOffset, 1.0f));
        spotLight.direction = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(trainLightDirection, 0.0f)));
    }

    void updateControlPoints(float deltaTime)
    {
        time += deltaTime * animationSpeed;

        for (size_t i = 0; i < controlPoints.size(); ++i)
        {
            controlPoints[i].y += animationAmplitude * sin(time + i * 0.5f);
        }
    }

    void drawObjects(Shader& shader) const
    {
        for (const auto& obj : gameObjects)
        {
            shader.setMat4("model", obj->transform.getModelMatrix());
            obj->model->Draw(shader);
        }
    }

    void drawLights(Shader& lightShader) const
    {
        for (const auto& light : pointLights)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), light.position);
            model = glm::scale(model, glm::vec3(0.2f));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", light.diffuse);
            sphereModel->Draw(lightShader);
        }
    }

    void drawTessellated(Shader& tessellationShader)
    {
        tessellationShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        glm::mat4 view = camera.getViewMatrix();
        tessellationShader.setMat4("projection", projection);
        tessellationShader.setMat4("view", view);
        tessellationShader.setMat4("model", bezierTransform.getModelMatrix());
        tessellationShader.setVec3("viewPos", camera.Position);
        tessellationShader.setFloat("tessLevel", tessLevel);
        setupShaderUniforms(tessellationShader);

        unsigned int vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec3), &controlPoints[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(vao);
        glPatchParameteri(GL_PATCH_VERTICES, 16); // 16 control points
        glDrawArrays(GL_PATCHES, 0, 16);
        glBindVertexArray(0);

		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
    }
};
