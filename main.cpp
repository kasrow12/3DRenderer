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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void updateWireFrame();
void generateLights();

int screenWidth = 1400;
int screenHeight = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));

float deltaTime = 0.0f;
double lastFrame = 0.0;

bool wireFrame = false;
bool captureMouse = false;
bool useBlinn = false;

std::vector<PointLight> pointLights;
SpotLight spotLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), 12.5f, 15.0f, 1.0f, 0.09f, 0.032f, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f));
DirLight dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f));

glm::vec3 skyColor(0.2f, 0.3f, 0.3f);

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

	Model ourModel("Assets/Objects/backpack/backpack.obj");

    generateLights();

    while (!glfwWindowShouldClose(window))
    {
		double currentFrame = glfwGetTime();
		deltaTime = (float)(currentFrame - lastFrame);
		lastFrame = currentFrame;

        processInput(window);

        glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --------- Shader uniforms
        shader.use();
        shader.setVec3("viewPos", camera.Position);
		shader.setBool("blinn", useBlinn);
        shader.setFloat("material.shininess", 32.0f);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		shader.setMat4("projection", projection);

		glm::mat4 view = camera.getViewMatrix();
		shader.setMat4("view", view);

		shader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)));

		// --------- Set lights
		dirLight.SetUniforms(shader);

		for (size_t i = 0; i < pointLights.size(); i++)
		{
			pointLights[i].SetUniforms(shader, i);
		}

		spotLight.SetUniforms(shader);

		// --------- Set fog
		shader.setFloat("fogSettings.maxDistance", 25.0f);
		shader.setFloat("fogSettings.minDistance", 1.0f);
		shader.setVec3("fogSettings.color", skyColor);

		// --------- Draw the model
        ourModel.Draw(shader);

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
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        // Point Lights
        if (ImGui::CollapsingHeader("Point Lights")) {
            for (size_t i = 0; i < pointLights.size(); i++) {
                if (ImGui::TreeNode(("Point Light " + std::to_string(i + 1)).c_str())) {
                    ImGui::SliderFloat3(("Position##" + std::to_string(i)).c_str(), &pointLights[i].position.x, -10.0f, 10.0f);
                    ImGui::SliderFloat(("Constant##" + std::to_string(i)).c_str(), &pointLights[i].constant, 0.0f, 1.0f);
                    ImGui::SliderFloat(("Linear##" + std::to_string(i)).c_str(), &pointLights[i].linear, 0.0f, 0.1f);
                    ImGui::SliderFloat(("Quadratic##" + std::to_string(i)).c_str(), &pointLights[i].quadratic, 0.0f, 0.1f);
                    ImGui::ColorEdit3(("Ambient##" + std::to_string(i)).c_str(), &pointLights[i].ambient.x);
                    ImGui::ColorEdit3(("Diffuse##" + std::to_string(i)).c_str(), &pointLights[i].diffuse.x);
                    ImGui::ColorEdit3(("Specular##" + std::to_string(i)).c_str(), &pointLights[i].specular.x);
                    ImGui::TreePop();
                }
            }
        }

        // Spot Light
        if (ImGui::CollapsingHeader("Spot Light")) {
            ImGui::SliderFloat3("Position", &spotLight.position.x, -10.0f, 10.0f);
            ImGui::SliderFloat3("Direction", &spotLight.direction.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Cut Off", &spotLight.cutOff, 0.0f, 90.0f);
            ImGui::SliderFloat("Outer Cut Off", &spotLight.outerCutOff, 0.0f, 90.0f);
            ImGui::SliderFloat("Constant", &spotLight.constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &spotLight.linear, 0.0f, 0.1f);
            ImGui::SliderFloat("Quadratic", &spotLight.quadratic, 0.0f, 0.1f);
            ImGui::ColorEdit3("Ambient", &spotLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &spotLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &spotLight.specular.x);
        }

        // Directional Light
        if (ImGui::CollapsingHeader("Directional Light")) {
            ImGui::SliderFloat3("Direction", &dirLight.direction.x, -10.0f, 10.0f);
            ImGui::ColorEdit3("Ambient", &dirLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &dirLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &dirLight.specular.x);
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
		camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);
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

    camera.processMouseMovement((float)xOffset, (float)yOffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!captureMouse)
		return;

	camera.processMouseScroll((float)yoffset);
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

void generateLights()
{
    pointLights.clear();
    pointLights.reserve(4);

    pointLights.emplace_back(glm::vec3(0.7f, 0.2f, 2.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f));
    pointLights.emplace_back(glm::vec3(2.3f, -3.3f, -4.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f));
    pointLights.emplace_back(glm::vec3(-4.0f, 2.0f, -12.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f));
    pointLights.emplace_back(glm::vec3(0.0f, 0.0f, -3.0f), 1.0f, 0.09f, 0.032f, glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f));
}