#pragma once

#include "mesh.h"
#include "utilities/imageLoader.hpp"
#include <iostream>
unsigned int generateBuffer(Mesh &mesh);

unsigned int generateTexture(PNGImage& image, bool useAlpha = false);