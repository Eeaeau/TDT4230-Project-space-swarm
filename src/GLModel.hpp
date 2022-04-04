#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include <vector>
#include <iostream>

#include "tiny_gltf.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLModel : public tinygltf::Model
{
public:
    GLModel() = default;
    GLModel(const char* filename, unsigned int instancing = 1, std::vector<glm::mat4> instanceMatrix = {});

    //tinygltf::Model model;
    std::vector<GLuint> bindModel();
    void drawModel();
    //void drawModel(GLuint vao);

private:
    bool loadModel(const char* filename);

    // Holds number of instances (if 1 the mesh will be rendered normally)
    unsigned int instancing; 
    std::vector<GLuint> vaos;
    GLuint VAO;
    //std::vector<glm::mat4> matricesMeshes;
    std::vector<glm::mat4> instanceMatrix;
    std::map<int, GLuint> bindMesh(std::map<int, GLuint> vbos,
        tinygltf::Model& model, tinygltf::Mesh& mesh);

    void bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
        tinygltf::Node& node);

    void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh);
    //void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, GLuint &VAO);

    void drawModelNodes(tinygltf::Model& model, tinygltf::Node& node);
};

