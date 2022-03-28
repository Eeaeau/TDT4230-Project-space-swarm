#pragma once

#include "mesh.h"
//#include "GLModel.hpp"
#include <glad/glad.h>
#include "utilities/imageLoader.hpp"
#include <iostream>

#include <GLFW/glfw3.h>


unsigned int generateBuffer(Mesh &mesh, const unsigned int amount = 1);

unsigned int generateTexture(PNGImage& image, bool useAlpha = false);

