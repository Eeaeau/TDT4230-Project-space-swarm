#include <glad/glad.h>
#include <program.hpp>
#include "glutils.h"
#include <vector>

template <class T>
unsigned int generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize) {
    GLuint bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
    glEnableVertexAttribArray(id);
    return bufferID;
}

unsigned int& generateBuffer(Mesh &mesh, unsigned int amount) {
    GLuint vaoID;

    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    generateAttribute(0, 3, mesh.vertices, false);
    generateAttribute(1, 3, mesh.normals, true);

    if (mesh.textureCoordinates.size() > 0) {
        generateAttribute(2, 2, mesh.textureCoordinates, false);
    }

    generateAttribute(3, 3, mesh.tangents, true);
    generateAttribute(4, 3, mesh.bitangents, true);

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);


    return vaoID;

}

//std::vector<GLuint> generateInctancedBuffer(Mesh &mesh, std::vector<glm::mat4> modelMatrices, const GLuint amount) {
//    std::vector<GLuint> VAOs;
//    
//    // vertex buffer object
//    unsigned int buffer;
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);
//
//    for (unsigned int i = 0; i < amount; i++)
//    {
//        
//        auto VAO = generateBuffer(mesh);
//        VAOs.push_back(VAO);
//
//        glBindVertexArray(VAO);
//        // vertex attributes
//        std::size_t vec4Size = sizeof(glm::vec4);
//        glEnableVertexAttribArray(3);
//        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
//        glEnableVertexAttribArray(4);
//        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
//        glEnableVertexAttribArray(5);
//        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
//        glEnableVertexAttribArray(6);
//        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
//
//        glVertexAttribDivisor(3, 1);
//        glVertexAttribDivisor(4, 1);
//        glVertexAttribDivisor(5, 1);
//        glVertexAttribDivisor(6, 1);
//
//        glBindVertexArray(0);
//    }
//
//    return VAOs;
//}

GLuint generateInctancedBuffer(Mesh &mesh, const std::vector<glm::mat4>& modelMatrices) {
 
    int amount = modelMatrices.size();

    

    std::size_t vec4Size = sizeof(glm::vec4);
    std::size_t mat4Size = sizeof(glm::mat4);
    
    int attribLoc = 6;
    int matDim = 4;
    // vertex buffer object
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * mat4Size, modelMatrices.data(), GL_STATIC_DRAW);


    auto VAO = generateBuffer(mesh);
    glBindVertexArray(VAO);
    // vertex attributes
    /*for (int locOffset = 0; locOffset < matDim; locOffset++) {
        glEnableVertexAttribArray(locOffset);
        glVertexAttribPointer(attribLoc + locOffset, matDim, GL_FLOAT, GL_FALSE, mat4Size, (void*)(locOffset * vec4Size));
        
        glVertexAttribDivisor(attribLoc + locOffset, 1);
    }*/
    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, matDim, GL_FLOAT, GL_FALSE, mat4Size, (void*)0);

    glEnableVertexAttribArray(attribLoc+1);
    glVertexAttribPointer(attribLoc+1, matDim, GL_FLOAT, GL_FALSE, mat4Size, (void*)(1 * vec4Size));

    glEnableVertexAttribArray(attribLoc + 2);
    glVertexAttribPointer(attribLoc + 2, matDim, GL_FLOAT, GL_FALSE, mat4Size, (void*)(2 * vec4Size));

    glEnableVertexAttribArray(attribLoc + 3);
    glVertexAttribPointer(attribLoc + 3, matDim, GL_FLOAT, GL_FALSE, mat4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(attribLoc, 1);
    glVertexAttribDivisor(attribLoc+1, 1);
    glVertexAttribDivisor(attribLoc+2, 1);
    glVertexAttribDivisor(attribLoc+3, 1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return VAO;
}

GLuint generateInctancedBuffer(Mesh& mesh, const std::vector<glm::vec3>& modelPos) {

    const GLuint amount = modelPos.size();
    auto VAO = generateBuffer(mesh);
    glBindVertexArray(VAO);

    int attribLoc = 5;
    int vecDim = 3;

    std::size_t vec3Size = sizeof(glm::vec3);

    // vertex buffer object
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::vec3), modelPos.data(), GL_STATIC_DRAW);

    // vertex attributes
    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, vecDim, GL_FLOAT, GL_FALSE, vec3Size, 0);

    glVertexAttribDivisor(attribLoc, 1);

    glBindVertexArray(0);

    return VAO;
}

GLuint generateInctancedBuffer2(Mesh& mesh, const std::vector<glm::mat4>& modelMatrices) {

    const GLuint amount = modelMatrices.size();
    auto VAO = generateBuffer(mesh);
    glBindVertexArray(VAO);

    int attribLoc = 6;
    int vecDim = 4;

    std::size_t vec4Size = sizeof(glm::vec4);

    // Shader Storage Buffer Object
    GLuint ssboModelMatrices;
    glGenBuffers(1, &ssboModelMatrices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboModelMatrices);

    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * modelMatrices.size(), modelMatrices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboModelMatrices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindVertexArray(0);

    return VAO;
}







unsigned int generateTexture(PNGImage &image, bool useAlpha) {
    
    GLuint textureID;

    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //std::unique_ptr<unsigned char*>  data = image.pixels.data();
    unsigned char* data = image.pixels.data();

    auto width = image.width;
    auto height = image.height;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  /*  if (data) {
        if (useAlpha) {
        }
        else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }*/

    //std::free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}