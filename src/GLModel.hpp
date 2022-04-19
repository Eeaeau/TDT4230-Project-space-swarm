#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utilities/shader.hpp>
//#include <vector>
#include <iostream>

#include "tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLModel : public tinygltf::Model
{
public:
    GLModel() = default;
    GLModel(const char* filename, unsigned int instancing = 1, std::vector<glm::mat4> instanceMatrices = {});

    //tinygltf::Model model;
    //std::vector<GLuint> bindModel();
    GLuint bindModel();
    
    void drawModel(Gloom::Shader *shader);
    //void drawModel(GLuint vao);

    void GLModel::updateInstanceMatrix(std::vector<glm::mat4> newInstanceMatrix);

    unsigned int instancing;
private:
    bool loadModel(const char* filename);


    //Gloom::Shader shader;
    // Holds number of instances (if 1 the mesh will be rendered normally)
    GLuint ssboModelMatrices;
    std::vector<GLuint> VAOs;
    GLuint VAO;
    std::map<int, GLuint> VBOs;

    std::map<int, GLuint> textureIDs;

    //std::vector<glm::mat4> matricesMeshes;
    std::vector<glm::mat4> instanceMatrices;
    std::map<int, GLuint> bindMesh(std::map<int, GLuint> vbos,
        tinygltf::Model& model, tinygltf::Mesh& mesh);

    void bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
        tinygltf::Node& node);

    void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, Gloom::Shader* shader);
    //void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, GLuint &VAO);

    void drawModelNodes(tinygltf::Model& model, tinygltf::Node& node, Gloom::Shader* shader);
};

