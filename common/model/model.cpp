#include "model.h"

unsigned int TextureFromFile(const char* path, const std::string& directory);

void Model::draw(Shader shader)
{
    for (Mesh mesh : meshes)
    {
    	mesh.draw(shader);
    }
}

void Model::loadModel(std::string path) 
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Error::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    //process all meshes contained in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    //do actions on all child nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        // process vecter position
        glm::vec3 vec;
        vec.x = mesh->mVertices[i].x;
        vec.y = mesh->mVertices[i].y;
        vec.z = mesh->mVertices[i].z;
        vertex.Position = vec;
        //process normal
        vec.x = mesh->mNormals[i].x;
        vec.y = mesh->mNormals[i].y;
        vec.z = mesh->mNormals[i].z;
        vertex.Normal = vec;
        //process Uv coords
        if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    //process textures
    if (mesh->mMaterialIndex >= 0)
    {
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Texture> specularMaps = loadMaterialTextures(material,
                aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); 
        }
    }
    return textures;
}

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

std::vector<Box> Model::getTheCollideBox(std::string type)
{
    std::vector<Box> boxes;
    if (type == "inner")
    {
        Box box;
        //meshes[0~15] are pillars. 2 meshes is a pillar
        for (int i = 0; i < 15; i += 2)
        {   
            bool newBox = true;
            std::vector<Vertex> vertices;
            vertices.insert(vertices.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
            vertices.insert(vertices.end(), meshes[i + 1].vertices.begin(), meshes[i+1].vertices.end());
            //find conner points
            for (const Vertex &vertex : vertices)
            {
                if (newBox)
                {
                    //initialize the box
                    box.minVec = vertex.Position;
                    box.maxVec = vertex.Position;
                    newBox = false;
                }
                else
                {
                    box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                    box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                    box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                    box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                    box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                    box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
                }
            }
            boxes.push_back(box);
        }

        //meshes 22 is the blue barrier
        bool newBox = true;
        //find conner points
        for (const Vertex& vertex : meshes[22].vertices)
        {
            if (newBox)
            {
                //initialize the box
                box.minVec = vertex.Position;
                box.maxVec = vertex.Position;
                newBox = false;
            }
            else
            {
                box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
            }
        }
        boxes.push_back(box);
        
        //meshes 33~35 is the middle wall with a hole, 33->wall, 34->hole, 35 steels, use 33 to build model
        newBox = true;
        //find wall Data
        for (const Vertex& vertex : meshes[33].vertices)
        {
            if (newBox)
            {
                //initialize the box
                box.minVec = vertex.Position;
                box.maxVec = vertex.Position;
                newBox = false;
            }
            else
            {
                box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
            }
        }
        Box wallData = box;
        //find hole Data
        newBox = true;
        
        for (const Vertex& vertex : meshes[34].vertices)
        {
            if (newBox)
            {
                //initialize the box
                box.minVec = vertex.Position;
                box.maxVec = vertex.Position;
                newBox = false;
            }
            else
            {
                box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
            }
        }
        Box holeData = box;
        Box box1, box2, box3, box4;
        box1.minVec = wallData.minVec;
        box1.maxVec = glm::vec3(wallData.maxVec.x, wallData.minVec.y, holeData.minVec.z);
        boxes.push_back(box1);

        box2.minVec = glm::vec3(wallData.minVec.x, wallData.maxVec.y, holeData.maxVec.z);
        box2.maxVec = wallData.maxVec;
        boxes.push_back(box2);

        box3.minVec = wallData.minVec;
        box3.maxVec = glm::vec3(wallData.maxVec.x, holeData.minVec.y, wallData.maxVec.z);
        boxes.push_back(box3);

        box4.minVec = glm::vec3(wallData.minVec.x, holeData.maxVec.y, wallData.minVec.z);
        box4.maxVec = wallData.maxVec;
        boxes.push_back(box4);
        //add hole
        newBox = true;
        //find conner points
        for (const Vertex& vertex : meshes[34].vertices)
        {
            if (newBox)
            {
                //initialize the box
                box.minVec = vertex.Position;
                box.maxVec = vertex.Position;
                newBox = false;
            }
            else
            {
                box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
            }
        }
        boxes.push_back(box);
        
        //meshes 36, 38, ..., 46 are six box barriers.
                //meshes[0~15] are pillars. 2 meshes is a pillar
        for (int i = 36; i < 47; i += 2)
        {
            newBox = true;
            //find conner points
            for (const Vertex& vertex : meshes[i].vertices)
            {
                if (newBox)
                {
                    //initialize the box
                    box.minVec = vertex.Position;
                    box.maxVec = vertex.Position;
                    newBox = false;
                }
                else
                {
                    box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                    box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                    box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                    box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                    box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                    box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
                }
            }
            boxes.push_back(box);
        }
    }
    else if (type == "outer")
    {
        Box box;
        //meshes[16~17] are walls, and 32 is the door
        bool newBox = true;
        std::vector<Vertex> vertices;
        vertices.insert(vertices.end(), meshes[16].vertices.begin(), meshes[16].vertices.end());
        vertices.insert(vertices.end(), meshes[17].vertices.begin(), meshes[17].vertices.end());
        vertices.insert(vertices.end(), meshes[32].vertices.begin(), meshes[32].vertices.end());

        //find conner points
        for (const Vertex& vertex : vertices)
        {
            if (newBox)
            {
                //initialize the box
                box.minVec = vertex.Position;
                box.maxVec = vertex.Position;
                newBox = false;
            }
            else
            {
                box.minVec.x = box.minVec.x < vertex.Position.x ? box.minVec.x : vertex.Position.x;
                box.minVec.y = box.minVec.y < vertex.Position.y ? box.minVec.y : vertex.Position.y;
                box.minVec.z = box.minVec.z < vertex.Position.z ? box.minVec.z : vertex.Position.z;

                box.maxVec.x = box.maxVec.x > vertex.Position.x ? box.maxVec.x : vertex.Position.x;
                box.maxVec.y = box.maxVec.y > vertex.Position.y ? box.maxVec.y : vertex.Position.y;
                box.maxVec.z = box.maxVec.z > vertex.Position.z ? box.maxVec.z : vertex.Position.z;
            }
        }
        boxes.push_back(box);
    }
    else
    {
        std::cout << "There is no collide box type: " << type << std::endl;
    }
    return boxes;
}