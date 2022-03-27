#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>

#include "tiny_gltf.h"

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::vector<glm::vec2> textureCoordinates;

    std::vector<unsigned int> indices;
};


bool loadModel(tinygltf::Model& model, const char* filename);

static std::string GetFilePathExtension(const std::string& FileName) {
    if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
    return "";
}