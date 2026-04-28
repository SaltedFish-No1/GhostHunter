#include "player.h"

#include <cmath>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "shaders.h"

float Player::jumpHeight(1.2f);
float Player::jumpSpeed(sqrt(2 * gravity * jumpHeight));

Player::Player(GLFWwindow* window, glm::vec3 pos) : Camera(window)
{
	viewPosition = glm::vec3(pos.x, pos.y + height, pos.z);
	Model::centralize(crosshairModel.meshes);
	Model::centralize(winEmoji.meshes);
	Model::centralize(loseEmoji.meshes);
}

Player::Player(GLFWwindow* window) : Camera(window)
{
}

void Player::processInput()
{
	processKeyEvent();
	processMouseMovement();
	defaultActions();
}

void Player::jump(float deltaTime)
{
	if (getPosition().y - ground < 1e-8)
	{
		onGround = false;
		velocity_y = jumpSpeed;
	}
}

void Player::fallProcessing()
{
	if (onGround == false)
	{
		float currentHeight = getPosition().y + velocity_y * timer.getDeltaTime() - 0.5f * gravity * timer.getDeltaTime() * timer.getDeltaTime();
		float newVelocity = velocity_y - timer.getDeltaTime() * gravity;

		if (currentHeight < ground && newVelocity < 0.0f)
		{
			viewPosition.y = ground + height;
			velocity_y = 0.0f;
			onGround = true;
		}
		else
		{
			velocity_y = newVelocity;
			viewPosition.y = currentHeight + height;
		}
	}
}

void Player::processKeyEvent()
{
	Camera::processKeyEvent();
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		jump(timer.getDeltaTime());
	}
}

void Player::defaultActions()
{
	fallProcessing();
	outerCollisionCheck();
	innerCollisionCheck();
	Camera::defaultActions();
}

void Player::setInnerBoxes(std::vector<Box> innerBoxes)
{
	this->innerBoxes = innerBoxes;
}

void Player::setOuterBoxes(std::vector<Box> outerBoxes)
{
	this->outerBoxes = outerBoxes;
	float groundHeight = outerBoxes[0].minVec.y;
	std::cout << "Ground Height: " << groundHeight << std::endl;
	viewPosition.y = groundHeight + height;
}

void Player::outerCollisionCheck()
{
	glm::vec3 maxBorder = outerBoxes[0].maxVec - outerCollisionOffset;
	glm::vec3 minBorder = outerBoxes[0].minVec + outerCollisionOffset;
	if (viewPosition.x > maxBorder.x)
	{
		viewPosition.x = maxBorder.x;
	}
	if (viewPosition.x < minBorder.x)
	{
		viewPosition.x = minBorder.x;
	}
	if (viewPosition.z > maxBorder.z)
	{
		viewPosition.z = maxBorder.z;
	}
	if (viewPosition.z < minBorder.z)
	{
		viewPosition.z = minBorder.z;
	}
}

void Player::innerCollisionCheck()
{
	bool isInner = false;
	for (const Box& box : innerBoxes)
	{
		glm::vec3 maxBorder = box.maxVec + innerCollisionOffset;
		glm::vec3 minBorder = box.minVec - innerCollisionOffset;
		if (viewPosition.x < maxBorder.x && viewPosition.x > minBorder.x &&
			viewPosition.z < maxBorder.z && viewPosition.z > minBorder.z)
		{
			isInner = true;
			if (getPosition().y > box.maxVec.y - 0.1f)
			{
				ground = box.maxVec.y;
			}
			else
			{
				float dist_x_min = viewPosition.x - minBorder.x;
				float dist_z_min = viewPosition.z - minBorder.z;
				float dist_x_max = maxBorder.x - viewPosition.x;
				float dist_z_max = maxBorder.z - viewPosition.z;
				if (dist_x_min < dist_z_min &&
					dist_x_min < dist_x_max &&
					dist_x_min < dist_z_max)
				{
					viewPosition.x = minBorder.x;
				}
				else if (dist_z_min < dist_x_min &&
					dist_z_min < dist_x_max &&
					dist_z_min < dist_z_max)
				{
					viewPosition.z = minBorder.z;
				}
				else if (dist_x_max < dist_x_min &&
					dist_x_max < dist_z_min &&
					dist_x_max < dist_z_max)
				{
					viewPosition.x = maxBorder.x;
				}
				else
				{
					viewPosition.z = maxBorder.z;
				}
			}
		}
	}
	if (!isInner)
	{
		ground = 0.0f;
		onGround = false;
	}
}

void Player::setViewPosition(glm::vec3 viewPosition)
{
	this->viewPosition = viewPosition;
}

void Player::setProjection(glm::mat4 projection)
{
	this->projectionMat4 = projection;
}

void Player::drawContainer(Shader shader)
{
	glEnable(GL_DEPTH_TEST);
	shader.use();

	glm::vec3 offset = glm::vec3(-0.11f, 1.8f, 0.1f);
	glm::vec3 scale = glm::vec3(0.0113546f);
	glm::mat4 modelMat4 = glm::mat4(1.0f);
	modelMat4 = glm::translate(modelMat4, offset);
	modelMat4 = glm::translate(modelMat4, getPosition());

	modelMat4 = glm::scale(modelMat4, glm::vec3(scale));
	modelMat4 = glm::rotate(modelMat4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	shader.setMat4("model", modelMat4);
	shader.setMat4("view", view);
	shader.setMat4("projection", projectionMat4);
	vacuumModel.meshes[0].draw(shader);
	vacuumModel.meshes[1].draw(shader);
	vacuumModel.meshes[2].draw(shader);
	vacuumModel.meshes[3].draw(shader);
}

void Player::drawVacuum(Shader shader)
{
	glEnable(GL_DEPTH_TEST);
	shader.use();

	glm::vec3 offset = glm::vec3(-395.929, 625.071, 501.315);
	glm::vec3 scale = glm::vec3(0.00242061);

	glm::mat4 modelMat4 = glm::mat4(1.0f);
	static glm::mat4 viewMat4 = view;

	modelMat4 = glm::scale(modelMat4, scale);
	modelMat4 = glm::rotate(modelMat4, glm::radians(225.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMat4 = glm::rotate(modelMat4, glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMat4 = glm::rotate(modelMat4, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	modelMat4 = glm::translate(modelMat4, offset);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		modelMat4 = glm::translate(modelMat4, glm::vec3(1, 0.4, 0.4) * static_cast<float>(std::sin(6.0 * glfwGetTime())));
	}

	shader.setMat4("model", modelMat4);
	shader.setMat4("view", viewMat4);
	shader.setMat4("projection", projectionMat4);

	vacuumModel.meshes[4].draw(shader);
	vacuumModel.meshes[5].draw(shader);
	vacuumModel.meshes[6].draw(shader);
}

void Player::drawCrosshair()
{
	static Shader shader("../res/shaders/crosshair.vertexshader", "../res/shaders/crosshair.fragmentshader");
	shader.use();
	crosshairModel.draw(shader);
}

void Player::drawEmoji(Shader shader, bool isWin)
{
	glEnable(GL_DEPTH_TEST);
	shader.use();

	shader.setBool("singleColor.isEnable", false);
	shader.setVec3("material.ambient", glm::vec3(0.8));
	shader.setVec3("material.diffuse", glm::vec3(diffuse));
	shader.setVec3("material.specular", glm::vec3(specular));
	shader.setFloat("material.shininess", 32.0f);

	static glm::vec3 currentViewPos = getViewPosition();
	static glm::mat4 viewMat4 = view;
	static glm::vec3 frontVec3 = getFront();

	glm::vec3 scale = glm::vec3(0.1f);
	glm::mat4 modelMat4 = glm::mat4(1.0f);
	modelMat4 = glm::translate(modelMat4, glm::vec3(0.15) * frontVec3 + glm::vec3(0.0f, 0.0f, 0.0f));
	modelMat4 = glm::translate(modelMat4, currentViewPos);
	modelMat4 = glm::scale(modelMat4, scale);
	modelMat4 = glm::rotate(modelMat4, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	static float currentTime = glfwGetTime();
	modelMat4 = glm::rotate(modelMat4, ((float)glfwGetTime() - currentTime) * glm::radians(90.0f), glm::vec3(0.5f, 0.5f, 0.5f));

	shader.setMat4("model", modelMat4);
	shader.setMat4("view", viewMat4);
	shader.setMat4("projection", projectionMat4);
	if (isWin)
	{
		winEmoji.draw(shader);
	}
	else
	{
		loseEmoji.draw(shader);
	}
}
