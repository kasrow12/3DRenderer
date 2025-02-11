#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <stb_image/stb_image.h>
#include <iostream>

#include "Source/Model.h"
#include "Source/Camera.h"
#include "Source/Shader.h"
#include "Source/PointLight.h"
#include "Source/SpotLight.h"
#include "Source/DirLight.h"

class Scene;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void updateWireFrame();
void setupScene(Scene& scene);

int screenWidth = 1400;
int screenHeight = 900;

float deltaTime = 0.0f;
double lastFrame = 0.0;

bool wireFrame = false;
bool captureMouse = false;
bool useBlinn = false;


glm::vec3 skyColor(0.2f, 0.3f, 0.3f);

glm::vec3 spotDirection(-1.0f, -0.25f, 0.0f);


class Transform
{
public:
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };

    glm::mat4 getModelMatrix() const
	{
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);
        return modelMatrix;
    }
};

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

class Scene
{
public:
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<PointLight> pointLights;
    SpotLight spotLight;
    DirLight dirLight;
	Model* sphereModel;
    float fogDistance = 60.0f;

	Camera camera = Camera(glm::vec3(0.0f, 1.0f, 15.0f));

    Scene() :
        spotLight(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), 12.5f, 15.0f, 1.0f, 0.008f, 0.001f,
            glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f)),
        dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f)) {
        generateLights();
    }

    void update(float deltaTime)
	{
		if (gameObjects.empty())
			return;

        gameObjects[0]->update(deltaTime);
        updateSpotlight();
    }

    void draw(Shader& shader, Shader& lightShader)
	{
        setupShaderUniforms(shader);
        drawObjects(shader);
		setupLightUniforms(lightShader);
        drawLights(lightShader);
    }

private:
    void generateLights()
	{
        pointLights.clear();
        pointLights.reserve(4);

        const float intensity = 0.8f;
        const glm::vec3 ambient(0.05f);
        const glm::vec3 specular(1.0f);

        std::vector<glm::vec3> positions = {
            glm::vec3(0.7f, 0.2f, 10.0f),
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
            pointLights.emplace_back(positions[i], 1.0f, 0.09f, 0.032f, ambient, colors[i], colors[i]);
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
        spotLight.direction = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(spotDirection, 0.0f)));
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
};

Scene scene;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "3DRenderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPos(window, (float)screenWidth / 2, (float)screenHeight / 2);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    stbi_set_flip_vertically_on_load(true);

    Shader shader("Assets/Shaders/vertex.vs", "Assets/Shaders/fragment.fs");
    Shader lightShader("Assets/Shaders/vertex.vs", "Assets/Shaders/lightFragment.fs");

	setupScene(scene);

    while (!glfwWindowShouldClose(window))
    {
		double currentFrame = glfwGetTime();
		deltaTime = (float)(currentFrame - lastFrame);
		lastFrame = currentFrame;

        processInput(window);

        glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.update(deltaTime);
		scene.draw(shader, lightShader);
        
		// --------- Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
		/*ImGui::Text("Camera Controls");
		ImGui::Text("WASD - Move Camera");
		ImGui::Text("Space - Move Up");
		ImGui::Text("Left Shift - Move Down");
		ImGui::Text("Control - Capture Mouse");
		ImGui::Text("Scroll - Zoom");*/
        if (ImGui::Checkbox("Wireframe (F)", &wireFrame))
        {
            updateWireFrame();
        }
		ImGui::Checkbox("Blinn (B)", &useBlinn);

        ImGui::SliderFloat("Fog Distance", &scene.fogDistance, 0.0f, 100.0f);
        ImGui::ColorEdit3("Sky Color", &skyColor.x);

		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        // Point Lights
        if (ImGui::CollapsingHeader("Point Lights")) 
        {
            for (size_t i = 0; i < scene.pointLights.size(); i++) 
            {
                if (ImGui::TreeNode(("Point Light " + std::to_string(i + 1)).c_str()))
                {
                    ImGui::SliderFloat3(("Position##" + std::to_string(i)).c_str(), &scene.pointLights[i].position.x, -10.0f, 10.0f);
                    ImGui::SliderFloat(("Constant##" + std::to_string(i)).c_str(), &scene.pointLights[i].constant, 0.0f, 1.0f);
                    ImGui::SliderFloat(("Linear##" + std::to_string(i)).c_str(), &scene.pointLights[i].linear, 0.0f, 0.1f);
                    ImGui::SliderFloat(("Quadratic##" + std::to_string(i)).c_str(), &scene.pointLights[i].quadratic, 0.0f, 0.1f);
                    ImGui::ColorEdit3(("Ambient##" + std::to_string(i)).c_str(), &scene.pointLights[i].ambient.x);
                    ImGui::ColorEdit3(("Diffuse##" + std::to_string(i)).c_str(), &scene.pointLights[i].diffuse.x);
                    ImGui::ColorEdit3(("Specular##" + std::to_string(i)).c_str(), &scene.pointLights[i].specular.x);
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("Directional Light"))
        {
            ImGui::SliderFloat3("Direction##", &scene.dirLight.direction.x, -10.0f, 10.0f);
            ImGui::ColorEdit3("Ambient##", &scene.dirLight.ambient.x);
            ImGui::ColorEdit3("Diffuse##", &scene.dirLight.diffuse.x);
            ImGui::ColorEdit3("Specular##", &scene.dirLight.specular.x);
        }

        if (ImGui::CollapsingHeader("Train Spot Light"))
        {
            ImGui::SliderFloat3("Train light direction", &spotDirection.x, -1.0f, 1.0f);
            ImGui::SliderFloat("Cut Off", &scene.spotLight.cutOff, 0.0f, 90.0f);
            ImGui::SliderFloat("Outer Cut Off", &scene.spotLight.outerCutOff, 0.0f, 90.0f);
            ImGui::SliderFloat("Constant", &scene.spotLight.constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &scene.spotLight.linear, 0.0f, 0.1f);
            ImGui::SliderFloat("Quadratic", &scene.spotLight.quadratic, 0.0f, 0.1f);
            ImGui::ColorEdit3("Ambient", &scene.spotLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &scene.spotLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &scene.spotLight.specular.x);
        }

        if (ImGui::CollapsingHeader("Objects"))
        {
            for (auto& obj : scene.gameObjects) 
            {
                if (ImGui::TreeNode(obj->name.c_str()))
                {
                    ImGui::SliderFloat3("Position", &obj->transform.position.x, -20.0f, 20.0f);
                    ImGui::SliderFloat3("Rotation", &obj->transform.rotation.x, 0.0f, 360.0f);
                    ImGui::SliderFloat3("Scale", &obj->transform.scale.x, 0.1f, 20.0f);
                    ImGui::TreePop();
                }
            }
        }

		// Train movement
		if (!scene.gameObjects.empty())
		{
			ImGui::Text("Train Movement");
			ImGui::Checkbox("Enable Movement", &scene.gameObjects[0]->isMoving);
			ImGui::SliderFloat("Movement Radius", &scene.gameObjects[0]->radius, 1.0f, 50.0f);
			ImGui::SliderFloat("Movement Speed", &scene.gameObjects[0]->speed, 0.1f, 2.0f);
		}

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

	// Camera controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		scene.camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scene.camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scene.camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scene.camera.processKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        scene.camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        scene.camera.processKeyboard(DOWN, deltaTime);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastX = (float)screenWidth / 2;
    static double lastY = (float)screenHeight / 2;
    static bool firstMouse = true;

	if (!captureMouse)
	{
		firstMouse = true;
		return;
	}

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xOffset = xpos - lastX;
    double yOffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    scene.camera.processMouseMovement((float)xOffset, (float)yOffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!captureMouse)
		return;

    scene.camera.processMouseScroll((float)yoffset);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
        wireFrame = !wireFrame;
		updateWireFrame();
	}
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		useBlinn = !useBlinn;
	}

	// Capture mouse on left control, cannot do this in processInput because this will be called only once
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
	{
		captureMouse = !captureMouse;
		if (captureMouse)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void updateWireFrame()
{
	if (wireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void setupScene(Scene& scene)
{
    Model* trainModel = new Model("Assets/Objects/GEVO/Gevo.obj", false);
    Model* sphereModel = new Model("Assets/Objects/basics/sphere.obj");
    Model* floorModel = new Model("Assets/Objects/basics/floor.obj");
    Model* trexModel = new Model("Assets/Objects/trex/trex.obj");

	scene.sphereModel = sphereModel;

    Transform trainTransform;
    trainTransform.scale = glm::vec3(1.0f);
    auto train = std::make_unique<GameObject>(trainModel, trainTransform, "Train");
    train->isMoving = true;
    scene.gameObjects.push_back(std::move(train));

    Transform floorTransform;
    floorTransform.scale = glm::vec3(100, 0.0001f, 100);
    scene.gameObjects.push_back(std::make_unique<GameObject>(floorModel, floorTransform, "Floor"));

    Transform sphereTransform;
    scene.gameObjects.push_back(std::make_unique<GameObject>(sphereModel, sphereTransform, "Sphere"));

    Transform trexTransform;
	trexTransform.position = glm::vec3(0.0f, -0.05f, 7.0f);
    scene.gameObjects.push_back(std::make_unique<GameObject>(trexModel, trexTransform, "T-rex"));
}