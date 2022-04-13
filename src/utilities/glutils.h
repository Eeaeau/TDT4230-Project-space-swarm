#pragma once

#include "mesh.h"
#include <glad/glad.h>
#include "utilities/imageLoader.hpp"
#include <iostream>

#include <GLFW/glfw3.h>


unsigned int generateBuffer(Mesh &mesh, const unsigned int amount = 1);

//std::vector<GLuint> generateInctancedBuffer(Mesh& mesh, std::vector<glm::mat4> modelMatrices, const GLuint amount = 1);
GLuint generateInctancedBuffer(Mesh& mesh, std::vector<glm::mat4> modelMatrices, const GLuint amount = 1);

unsigned int generateTexture(PNGImage& image, bool useAlpha = false);
