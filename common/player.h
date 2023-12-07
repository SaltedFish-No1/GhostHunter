#pragma once
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <model/model.h>

//Add jump function
class Player : public Camera
{
//protected:
public:
	static float jumpHeight;
	static float jumpSpeed; 
	float height = 2.0f; //viewpoint.y = position.y + height
	

	//collision detection related
	float innerCollisionOffset = 0.4f;
	float outerCollisionOffset = 1.0f;

	std::vector<Box> innerBoxes;
	std::vector<Box> outerBoxes;

	//jump related
	bool onGround = true;
	float ground = 0.0f;
	float velocity_y = 0.0f;
	void jump(float deltaTime);
	void fallProcessing();
	//others
	void processKeyEvent();
	void defaultActions();

public:
	Player(GLFWwindow* window, glm::vec3 pos) :Camera(window)
	{
		viewPosition = glm::vec3(pos.x, pos.y+height, pos.z);
		Model::centralize(crosshairModel.meshes);
		Model::centralize(winEmoji.meshes);
		Model::centralize(loseEmoji.meshes);
	}
	Player(GLFWwindow* window) :Camera(window)
	{

	}
	//things that camera won't do
	void processInput();
	inline glm::vec3 getPosition()
	{
		return glm::vec3(viewPosition.x, viewPosition.y - height, viewPosition.z);
	}
	inline glm::vec3 getViewPosition()
	{
		return viewPosition;
	}


	//collision detection related
	void setInnerBoxes(std::vector<Box> innerBoxes)
	{
		this->innerBoxes = innerBoxes;
	}
	void setOuterBoxes(std::vector<Box> outerBoxes)
	{
		this->outerBoxes = outerBoxes;
		float groundHeight = outerBoxes[0].minVec.y;
		std::cout << "Ground Height: " << groundHeight << std::endl;
		viewPosition.y = groundHeight + height;
	}
	void outerCollisionCheck()
	{
		
		//2 dimmensional check, only check the xz plane
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
	void innerCollisionCheck()
	{
		bool isInner = false;
		for (const Box &box : innerBoxes)
		{
			
			glm::vec3 maxBorder = box.maxVec + innerCollisionOffset;
			glm::vec3 minBorder = box.minVec - innerCollisionOffset;
			//see if the player already loacted inside of the inner box
			if (viewPosition.x < maxBorder.x && viewPosition.x > minBorder.x &&
				viewPosition.z < maxBorder.z && viewPosition.z > minBorder.z)
				//check the xz plane
			{
				isInner = true;
				//player is in inner box
				//1. which line is the closest
				//2. project the xz position on that line 
				//3. update the projected point as new position
				if (getPosition().y > box.maxVec.y - 0.1f) //player actually "float" on the surface, avoid "teleport" when player fall
				{
					//jump up to the inner object
					ground = box.maxVec.y;
				}
				else
				{
					//collide with the inner object
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
		if(!isInner)
		{
			//leave the inner object area
			ground = 0.0f;
			onGround = false;
		}

	}

//protected:
	//draw related
	//Model related
	Model vacuumModel = Model("../res/model/vacuum/vacuum.obj");
	Model crosshairModel = Model("../res/model/sphere/sphere.obj");
	Model winEmoji = Model("../res/model/emoji/WinEmoji.obj");
	Model loseEmoji = Model("../res/model/emoji/loseEmoji.obj");
	//glm::mat4 viewMat4 = glm::mat4(1.0f);
	glm::mat4 projectionMat4 = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.01f, 200.0f);
	//light settting
	float ambient = 0.1f;
	float diffuse = 0.6f;
	float specular = 0.0f;
	float alpha = 1.0f;



public:
	void setViewPosition(glm::vec3 viewPosition)
	{
		this->viewPosition = viewPosition;
	}
	void setProjection(glm::mat4 projection)
	{
		this->projectionMat4 = projection;
	}

	void drawContainer(Shader shader)
	{
		glEnable(GL_DEPTH_TEST);
		shader.use();
		/**
		* Still need light shadering setting
		*/
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
		//vacuumModel.draw(shader);
		vacuumModel.meshes[0].draw(shader);
		vacuumModel.meshes[1].draw(shader);
		vacuumModel.meshes[2].draw(shader);
		vacuumModel.meshes[3].draw(shader);
	}
	void drawVacuum(Shader shader)
	{
		glEnable(GL_DEPTH_TEST);
		shader.use();
		/**
		* Still need light shadering setting
		*/

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
			modelMat4 = glm::translate(modelMat4, glm::vec3(1, 0.4, 0.4) * (std::sinf(6 * glfwGetTime())));
		}
		

		shader.setMat4("model", modelMat4);
		shader.setMat4("view", viewMat4);
		shader.setMat4("projection", projectionMat4);
		

		vacuumModel.meshes[4].draw(shader);
		vacuumModel.meshes[5].draw(shader);
		vacuumModel.meshes[6].draw(shader);
	}

	void drawCrosshair()
	{
		static Shader shader("../res/shaders/crosshair.vertexshader", "../res/shaders/crosshair.fragmentshader");
		shader.use();
		crosshairModel.draw(shader);
	}

	void drawEmoji(Shader shader, bool isWin)
	{
		glEnable(GL_DEPTH_TEST);
		shader.use();
		/**
		* light shadering setting
		*/
		shader.setBool("singleColor.isEnable", false);
		shader.setVec3("material.ambient", glm::vec3(0.8));
		shader.setVec3("material.diffuse", glm::vec3(diffuse));
		shader.setVec3("material.specular", glm::vec3(specular));
		shader.setFloat("material.shininess", 32.0f);

		//freeze the player
		static glm::vec3 currentViewPos = getViewPosition();
		static glm::mat4 viewMat4 = view;
		static glm::vec3 frontVec3 = getFront();


		glm::vec3 scale = glm::vec3(0.1f);
		glm::mat4 modelMat4 = glm::mat4(1.0f);
		modelMat4 = glm::translate(modelMat4, glm::vec3(0.15) * frontVec3 + glm::vec3(0.0f, 0.0f, 0.0f));
		modelMat4 = glm::translate(modelMat4, currentViewPos);
		modelMat4 = glm::scale(modelMat4, scale);
		//modelMat4 = glm::rotate(modelMat4, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMat4 = glm::rotate(modelMat4, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//keep rotating
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
};

