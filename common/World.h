#pragma once
#include <model/model.h>
#include <Player.h>

class World :public Model
{
	//Model related
	glm::vec3 offset = glm::vec3(0.0f, 3.5630004f, 0.0f); //offset of the model->ground = height
	glm::vec3 scale = glm::vec3(0.6f);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.1f, 200.0f);
	//light settting
	float ambient = 0.05f;
	float diffuse = 0.6f;
	float specular = 0.0f;
	float alpha = 1.0f;
	//collision detection related
	std::vector<Box> innerBoxes;
	std::vector<Box> outerBoxes;
	

public:
	World(std::string directory) :Model(directory)
	{
		innerBoxes = getTheCollideBox("inner");
		outerBoxes = getTheCollideBox("outer");
	}

	void setModel(glm::mat4 model)
	{
		this->model = model;
		this->model = glm::translate(this->model, offset); // translate the background so the ground is at y = 0 
		this->model = glm::scale(this->model, scale);	// it's a bit too big for our scene, so scale it down
	}

	void setView(glm::mat4 view)
	{
		this->view = view;
	}

	void setProjection(glm::mat4 projection)
	{
		this->projection = projection;
	}

	void setScale(float scale)
	{
		this->scale = glm::vec3(scale);
	}

	void setOffset(glm::vec3 scale)
	{
		this->offset = offset;
	}

	void draw(Shader shader, Player player)
	{
		glEnable(GL_DEPTH_TEST);
		shader.use();
		/**
		* light shadering settings
		*/
		shader.setBool("singleColor.isEnable", false);
		shader.setVec3("material.ambient", glm::vec3(ambient));
		shader.setVec3("material.diffuse", glm::vec3(diffuse));
		shader.setVec3("material.specular", glm::vec3(specular));
		shader.setFloat("material.shininess", 32.0f);

		//set light
		shader.setVec3("torch.position", player.getViewPosition());
		shader.setVec3("torch.direction", player.getFront());
		shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
		shader.setFloat("torch.constant", 1.0f);
		shader.setFloat("torch.linear", 0.09f);
		shader.setFloat("torch.quadratic", 0.032f);

		//model settings
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		Model::draw(shader);
	}

	//this function is wrong, can't draw boxes
	void drawCollisionBoxes(Shader shader)
	{
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Using Wireframe Mode


		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		//preparing the drawing data
		unsigned int VAO;
		unsigned int VBO;
		
		for (const Box &box : innerBoxes)
		{
			glm::vec3 p1 = box.minVec; // 左下后顶点
			glm::vec3 p2 = box.maxVec; // 右上前顶点
			glm::vec3 p3(p1.x, p1.y, p2.z); // 左下前顶点
			glm::vec3 p4(p1.x, p2.y, p1.z); // 左上后顶点
			glm::vec3 p5(p2.x, p1.y, p1.z); // 右下后顶点
			glm::vec3 p6(p1.x, p2.y, p2.z); // 左上前顶点
			glm::vec3 p7(p2.x, p1.y, p2.z); // 右下前顶点
			glm::vec3 p8(p2.x, p2.y, p1.z); // 右上后顶点

			std::vector<glm::vec3> vertices;
			vertices.push_back(p3); // p3
			vertices.push_back(p7); // p7
			vertices.push_back(p2); // p2
			vertices.push_back(p3); // p3
			vertices.push_back(p6); // p6
			vertices.push_back(p2); // p2
			// 后面
			vertices.push_back(p1); // p1
			vertices.push_back(p5); // p5
			vertices.push_back(p8); // p8
			vertices.push_back(p1); // p1
			vertices.push_back(p8); // p8
			vertices.push_back(p4); // p4
			// 左面
			vertices.push_back(p1); // p1
			vertices.push_back(p4); // p4
			vertices.push_back(p6); // p6
			vertices.push_back(p1); // p1
			vertices.push_back(p6); // p6
			vertices.push_back(p3); // p3
			// 右面
			vertices.push_back(p2); // p2
			vertices.push_back(p8); // p8
			vertices.push_back(p5); // p5
			vertices.push_back(p2); // p2
			vertices.push_back(p5); // p5
			vertices.push_back(p7); // p7
			// 上面
			vertices.push_back(p4); // p4
			vertices.push_back(p8); // p8
			vertices.push_back(p2); // p2
			vertices.push_back(p4); // p4
			vertices.push_back(p2); // p2
			vertices.push_back(p6); // p6
			// 下面
			vertices.push_back(p1); // p1
			vertices.push_back(p3); // p3
			vertices.push_back(p7); // p7
			vertices.push_back(p1); // p1
			vertices.push_back(p7); // p7
			vertices.push_back(p5); // p5



			glGenVertexArrays(1, &VAO);  //Gen & Bind VAO(glGenVertexArrays(numberOfVAOs, VAOs))
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO); // Gen & Bind VBO
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
			//add position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//Set the vertex attribute pointer
			glEnableVertexAttribArray(0);


			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	//getInnerBoxes() and getOuterBoxes() must after calling setModel() 
	std::vector<Box> getInnerBoxes()
	{
		//process innerBoxes, excute "model" things on it.
		std::vector<Box> processedInnerBoxes;
		for (const Box &box : innerBoxes)
		{
			Box processedBox;
			processedBox.maxVec = glm::vec3(model * glm::vec4(box.maxVec, 1.0f));
			processedBox.minVec = glm::vec3(model * glm::vec4(box.minVec, 1.0f));
			processedInnerBoxes.push_back(processedBox);
		}

		return processedInnerBoxes;
		return innerBoxes;
	}

	//getInnerBoxes() and getOuterBoxes() must after calling setModel() 
	std::vector<Box> getOuterBoxes()
	{
		//process outerBoxes, excute "model" things on it.
		std::vector<Box> processedOuterBoxes;
		for (const Box &box : outerBoxes)
		{
			Box processedBox;
			processedBox.maxVec = glm::vec3(model * glm::vec4(box.maxVec, 1.0f));
			processedBox.minVec = glm::vec3(model * glm::vec4(box.minVec, 1.0f));
			processedOuterBoxes.push_back(processedBox);
		}

		return processedOuterBoxes;
	}
};