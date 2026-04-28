#include "World.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"
#include "shaders.h"

World::World(std::string directory) : Model(directory)
{
	innerBoxes = getTheCollideBox("inner");
	outerBoxes = getTheCollideBox("outer");
}

void World::setModel(glm::mat4 model)
{
	this->model = model;
	this->model = glm::translate(this->model, offset);
	this->model = glm::scale(this->model, scale);
}

void World::setView(glm::mat4 view)
{
	this->view = view;
}

void World::setProjection(glm::mat4 projection)
{
	this->projection = projection;
}

void World::setScale(float scale)
{
	this->scale = glm::vec3(scale);
}

void World::setOffset(glm::vec3 newOffset)
{
	this->offset = newOffset;
}

void World::draw(Shader shader, Player player)
{
	glEnable(GL_DEPTH_TEST);
	shader.use();

	shader.setBool("singleColor.isEnable", false);
	shader.setVec3("material.ambient", glm::vec3(ambient));
	shader.setVec3("material.diffuse", glm::vec3(diffuse));
	shader.setVec3("material.specular", glm::vec3(specular));
	shader.setFloat("material.shininess", 32.0f);

	shader.setVec3("torch.position", player.getViewPosition());
	shader.setVec3("torch.direction", player.getFront());
	shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
	shader.setFloat("torch.constant", 1.0f);
	shader.setFloat("torch.linear", 0.09f);
	shader.setFloat("torch.quadratic", 0.032f);

	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	Model::draw(shader);
}

void World::drawCollisionBoxes(Shader shader)
{
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	unsigned int VAO;
	unsigned int VBO;

	for (const Box& box : innerBoxes)
	{
		glm::vec3 p1 = box.minVec;
		glm::vec3 p2 = box.maxVec;
		glm::vec3 p3(p1.x, p1.y, p2.z);
		glm::vec3 p4(p1.x, p2.y, p1.z);
		glm::vec3 p5(p2.x, p1.y, p1.z);
		glm::vec3 p6(p1.x, p2.y, p2.z);
		glm::vec3 p7(p2.x, p1.y, p2.z);
		glm::vec3 p8(p2.x, p2.y, p1.z);

		std::vector<glm::vec3> vertices;
		vertices.push_back(p3);
		vertices.push_back(p7);
		vertices.push_back(p2);
		vertices.push_back(p3);
		vertices.push_back(p6);
		vertices.push_back(p2);

		vertices.push_back(p1);
		vertices.push_back(p5);
		vertices.push_back(p8);
		vertices.push_back(p1);
		vertices.push_back(p8);
		vertices.push_back(p4);

		vertices.push_back(p1);
		vertices.push_back(p4);
		vertices.push_back(p6);
		vertices.push_back(p1);
		vertices.push_back(p6);
		vertices.push_back(p3);

		vertices.push_back(p2);
		vertices.push_back(p8);
		vertices.push_back(p5);
		vertices.push_back(p2);
		vertices.push_back(p5);
		vertices.push_back(p7);

		vertices.push_back(p4);
		vertices.push_back(p8);
		vertices.push_back(p2);
		vertices.push_back(p4);
		vertices.push_back(p2);
		vertices.push_back(p6);

		vertices.push_back(p1);
		vertices.push_back(p3);
		vertices.push_back(p7);
		vertices.push_back(p1);
		vertices.push_back(p7);
		vertices.push_back(p5);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

std::vector<Box> World::getInnerBoxes()
{
	std::vector<Box> processedInnerBoxes;
	for (const Box& box : innerBoxes)
	{
		Box processedBox;
		processedBox.maxVec = glm::vec3(model * glm::vec4(box.maxVec, 1.0f));
		processedBox.minVec = glm::vec3(model * glm::vec4(box.minVec, 1.0f));
		processedInnerBoxes.push_back(processedBox);
	}
	return processedInnerBoxes;
}

std::vector<Box> World::getOuterBoxes()
{
	std::vector<Box> processedOuterBoxes;
	for (const Box& box : outerBoxes)
	{
		Box processedBox;
		processedBox.maxVec = glm::vec3(model * glm::vec4(box.maxVec, 1.0f));
		processedBox.minVec = glm::vec3(model * glm::vec4(box.minVec, 1.0f));
		processedOuterBoxes.push_back(processedBox);
	}
	return processedOuterBoxes;
}
