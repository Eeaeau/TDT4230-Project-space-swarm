#pragma once

#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <vector>
#include <iostream>



#include "tiny_gltf.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLModel : public tinygltf::Model
{
public:
    //GLModel::GLModel() {
    //    //this->model = model;
    //}
    //tinygltf::Model model;
    GLuint bindModel();
    void drawModel(GLuint vao);

private:
    std::map<int, GLuint> bindMesh(std::map<int, GLuint> vbos,
        tinygltf::Model& model, tinygltf::Mesh& mesh);

    void bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
        tinygltf::Node& node);

    void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh);

    void drawModelNodes(tinygltf::Model& model, tinygltf::Node& node);
};

