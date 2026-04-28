#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "camera.h"
#include <model/model.h>

class Shader;

class Player : public Camera
{
public:
	static float jumpHeight;
	static float jumpSpeed;
	float height = 2.0f;

	float innerCollisionOffset = 0.4f;
	float outerCollisionOffset = 1.0f;

	std::vector<Box> innerBoxes;
	std::vector<Box> outerBoxes;

	bool onGround = true;
	float ground = 0.0f;
	float velocity_y = 0.0f;
	void jump(float deltaTime);
	void fallProcessing();

	void processKeyEvent();
	void defaultActions();

	Player(GLFWwindow* window, glm::vec3 pos);
	Player(GLFWwindow* window);

	void processInput();

	inline glm::vec3 getPosition()
	{
		return glm::vec3(viewPosition.x, viewPosition.y - height, viewPosition.z);
	}
	inline glm::vec3 getViewPosition()
	{
		return viewPosition;
	}

	void setInnerBoxes(std::vector<Box> innerBoxes);
	void setOuterBoxes(std::vector<Box> outerBoxes);
	void outerCollisionCheck();
	void innerCollisionCheck();

	Model vacuumModel = Model("../res/model/vacuum/vacuum.obj");
	Model crosshairModel = Model("../res/model/sphere/sphere.obj");
	Model winEmoji = Model("../res/model/emoji/WinEmoji.obj");
	Model loseEmoji = Model("../res/model/emoji/LoseEmoji.obj");
	glm::mat4 projectionMat4 = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.01f, 200.0f);

	float ambient = 0.1f;
	float diffuse = 0.6f;
	float specular = 0.0f;
	float alpha = 1.0f;

	void setViewPosition(glm::vec3 viewPosition);
	void setProjection(glm::mat4 projection);

	void drawContainer(Shader shader);
	void drawVacuum(Shader shader);
	void drawCrosshair();
	void drawEmoji(Shader shader, bool isWin);
};
