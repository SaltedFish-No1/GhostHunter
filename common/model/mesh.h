#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <shaders.h>
#include<glad/glad.h>
#include <sstream>

struct Vertex
{
    glm::vec3 Position;
    glm::vec2 Normal;
    glm::vec2  TexCoords;
    //debug function
    std::string toString()
    {
        std::stringstream ss;
        ss << Position.x << " " << Position.y << " " << Position.z << " ";
        ss << Normal.x << " " << Normal.y << " ";
        ss << TexCoords.x << " " << TexCoords.y << " ";
        return ss.str();
    }
};

struct Texture
{
    unsigned int id;
    std::string type; //specular or diffuse
    std::string path;  // 我们储存纹理的路径用于与其它纹理进行比较
};

class Mesh
{
public:
    /* mesh data */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    /* functions */
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void draw(Shader shader);
    /* render data */
    unsigned int VAO, VBO, EBO;
    /* functions */
    void setupMesh();
};