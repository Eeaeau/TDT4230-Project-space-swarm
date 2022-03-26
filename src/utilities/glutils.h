#pragma once

#include "mesh.h"
#include <glad/glad.h>
#include "utilities/imageLoader.hpp"
#include <iostream>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

unsigned int generateBuffer(Mesh &mesh);

unsigned int generateTexture(PNGImage& image, bool useAlpha = false);

std::map<int, GLuint> bindMesh(std::map<int, GLuint> vbos,
    tinygltf::Model& model, tinygltf::Mesh& mesh);

void bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
    tinygltf::Node& node);

GLuint bindModel(tinygltf::Model& model);

void drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh);

void drawModelNodes(tinygltf::Model& model, tinygltf::Node& node);

void drawModel(GLuint vao, tinygltf::Model& model);