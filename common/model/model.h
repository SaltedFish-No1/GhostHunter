#pragma once
#include <iostream>
#include "shaders.h"
#include "mesh.h"
#include <tools.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //load texture
#endif // !STB_IMAGE_IMPLEMENTATION


class Model
{
public:
    Model(std::string path)
    {
        loadModel(path);
    }

    void draw(Shader shader);
    std::vector<Box> getTheCollideBox(std::string type);

    //make the model center in (0, 0, 0), return the offset
    static void centralize(std::vector<Mesh>& meshes, std::vector<int> indices = {})
    {
        //indices = {} -> take all
        if (indices.size() == 0)
        {
            for (int i = 0; i != meshes.size(); ++i)
            {
                indices.push_back(i);
            }
        }

        bool firstTime = true;
        glm::vec3 vec_min, vec_max;
        for (int i : indices)
        {
            for (const Vertex& vertex : meshes[i].vertices)
            {
                if (firstTime)
                {
                    //initialize the vec_max and vec_min
                    vec_min = vertex.Position;
                    vec_max = vertex.Position;
                    firstTime = false;
                }
                else
                {
                    vec_min.x = vertex.Position.x < vec_min.x ? vertex.Position.x : vec_min.x;
                    vec_min.y = vertex.Position.y < vec_min.y ? vertex.Position.y : vec_min.y;
                    vec_min.z = vertex.Position.z < vec_min.z ? vertex.Position.z : vec_min.z;
                    vec_max.x = vertex.Position.x > vec_max.x ? vertex.Position.x : vec_max.x;
                    vec_max.y = vertex.Position.y > vec_max.y ? vertex.Position.y : vec_max.y;
                    vec_max.z = vertex.Position.z > vec_max.z ? vertex.Position.z : vec_max.z;
                }
            }
        }
        
        glm::vec3 offset = glm::vec3(-0.5) * (vec_max + vec_min);

        //try to change it directly
        for (int i : indices)
        {
            for (Vertex& vertex : meshes[i].vertices)
            {
                //std::cout << vertex.Position.x << " " << vertex.Position.y << " " << vertex.Position.z << std::endl;
                vertex.Position += offset;
                //std::cout << vertex.Position.x << " " << vertex.Position.y << " " << vertex.Position.z << std::endl;
            }
            meshes[i].setupMesh();
        }
    }

//protected:
    /*  模型数据  */
    std::vector <Mesh> meshes;
    std::string directory;
    // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Texture> textures_loaded;	
    /*  函数   */
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        std::string typeName);
};

