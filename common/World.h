#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <model/model.h>

class Player;
class Shader;

class World : public Model
{
	glm::vec3 offset = glm::vec3(0.0f, 3.5630004f, 0.0f);
	glm::vec3 scale = glm::vec3(0.6f);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.1f, 200.0f);

	float ambient = 0.05f;
	float diffuse = 0.6f;
	float specular = 0.0f;
	float alpha = 1.0f;

	std::vector<Box> innerBoxes;
	std::vector<Box> outerBoxes;

public:
	explicit World(std::string directory);

	void setModel(glm::mat4 model);
	void setView(glm::mat4 view);
	void setProjection(glm::mat4 projection);
	void setScale(float scale);
	void setOffset(glm::vec3 newOffset);

	void draw(Shader shader, Player player);
	void drawCollisionBoxes(Shader shader);

	std::vector<Box> getInnerBoxes();
	std::vector<Box> getOuterBoxes();
};
