#pragma once

#include <utilities/window.hpp>
#include "sceneGraph.hpp"
#include <stdio.h>
#include <stdlib.h>


void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initGame(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);

