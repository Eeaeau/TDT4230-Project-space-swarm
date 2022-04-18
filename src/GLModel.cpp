#include "GLModel.hpp"


//  ------------------------------------------------------ //
//  ------ Code is based on tinygltf loader example ------ //  
//  ------------------------------------------------------ //




void linkAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    //glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLModel::GLModel(const char* filename, unsigned int instancing, std::vector<glm::mat4> instanceMatrices)
{
    loadModel(filename);
    this->instancing = instancing;
    this->instanceMatrices = instanceMatrices;
    //this->VAOs = std::vector<GLuint>(instancing);
    bindModel();
    std::cout << "ehm" << std::endl;
}

bool GLModel::loadModel(const char* filename)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(this, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}


std::map<int, GLuint> GLModel::bindMesh(std::map<int, GLuint> vbos,
    tinygltf::Model& model, tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView& bufferView = model.bufferViews[i];
        if (bufferView.target == 0) {  // TODO impl drawarrays
            std::cout << "WARN: bufferView.target is zero" << std::endl;
            continue;  // Unsupported bufferView.
                       /*
                         From spec2.0 readme:
                         https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
                                  ... drawArrays function should be used with a count equal to
                         the count            property of any of the accessors referenced by the
                         attributes            property            (they are all equal for a given
                         primitive).
                       */
        }

        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        std::cout << "bufferview.target " << bufferView.target << std::endl;

        GLuint vbo;
        glGenBuffers(1, &vbo);
        vbos[i] = vbo;
        glBindBuffer(bufferView.target, vbo);

        std::cout << "buffer.data.size = " << buffer.data.size()
            << ", bufferview.byteOffset = " << bufferView.byteOffset
            << std::endl;

        glBufferData(bufferView.target, bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    }

    

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        for (auto& attrib : primitive.attributes) {
            tinygltf::Accessor accessor = model.accessors[attrib.second];
            int byteStride =
                accessor.ByteStride(model.bufferViews[accessor.bufferView]);
            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int vaa = -1;
            if (attrib.first.compare("POSITION") == 0) vaa = 0;
            if (attrib.first.compare("NORMAL") == 0) vaa = 1;
            if (attrib.first.compare("TANGENT") == 0) vaa = 2;
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 3;
            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE,
                    byteStride, BUFFER_OFFSET(accessor.byteOffset));

                if (instancing > 1) {
                    // Shader Storage Buffer Object
                    glGenBuffers(1, &ssboModelMatrices);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboModelMatrices);

                    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * instanceMatrices.size(), instanceMatrices.data(), GL_STATIC_DRAW);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboModelMatrices);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                }

            }
            else
                std::cout << "vaa missing: " << attrib.first << std::endl;

        }

        if (model.textures.size() > 0) {
            // fixme: Use material's baseColor

            //for (auto& tex : model.textures) {
            for (size_t i = 0; i < model.textures.size(); ++i) {

                auto tex = model.textures[i];

                if (tex.source > -1) {

                    //auto texid = &textureIDs[tex.source];
                    auto texid = &textureIDs[i];
                    glGenTextures(1, texid);

                    tinygltf::Image& image = model.images[tex.source];

                    glBindTexture(GL_TEXTURE_2D, *texid);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                    GLenum format = GL_RGBA;

                    if (image.component == 1) {
                        format = GL_RED;
                    }
                    else if (image.component == 2) {
                        format = GL_RG;
                    }
                    else if (image.component == 3) {
                        format = GL_RGB;
                    }
                    else {
                        //format 
                    }

                    GLenum type = GL_UNSIGNED_BYTE;
                    if (image.bits == 8) {
                        // ok
                    }
                    else if (image.bits == 16) {
                        type = GL_UNSIGNED_SHORT;
                    }
                    else {
                        type = GL_FLOAT;
                    }

                    GLint internalFormat = GL_RGBA;
                    if (image.pixel_type == 5126) {
                        internalFormat = GL_RGBA16F;
                    }

                    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0,
                        format, type, &image.image.at(0));
                
                    // Generates MipMaps
                    glGenerateMipmap(GL_TEXTURE_2D);

                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
            //tinygltf::Texture& tex = model.textures[0];

        }
    }

    return vbos;
}

void GLModel::bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
    tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {

        //VAOs.push_back(VAO);

        bindMesh(vbos, model, model.meshes[node.mesh]);
    }

    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
        bindModelNodes(vbos, model, model.nodes[node.children[i]]);
    }
}

GLuint GLModel::bindModel() {
    /*for (size_t i = 0; i < vaos.size(); ++i) {
    }*/
    //VAOs = std::vector<GLuint>();

    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    const tinygltf::Scene& scene = this->scenes[this->defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < this->nodes.size()));
        bindModelNodes(VBOs, *this, this->nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
    // cleanup vbos
    for (size_t i = 0; i < VBOs.size(); ++i) {
        glDeleteBuffers(1, &VBOs[i]);
    }
    return this->VAO;
}


//void GLModel::drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, GLuint &VAO) {
void GLModel::drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, Gloom::Shader* shader) {
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        //auto primitiveMat = &materials[primitive.material];
        //
        //auto baseColor = primitiveMat->pbrMetallicRoughness.baseColorTexture.index;
        //int useNormalTexture = -1;
        //auto normalMap = primitiveMat->normalTexture.index;
        //auto emissiveFactor = primitiveMat->emissiveFactor;
        //auto roughnessMap = primitiveMat->pbrMetallicRoughness.metallicRoughnessTexture.index;
        //auto roughnessFactor = primitiveMat->pbrMetallicRoughness.roughnessFactor;
        //

        ///*for (unsigned int i = 0; i < textures.size(); i++) {


        //}*/
        ///*for (unsigned int i = 0; i < materials.size(); i++) {

        //}*/
        //glBindTextureUnit(0, textureIDs[baseColor]);
        //glBindTextureUnit(1, textureIDs[normalMap]);
        //glBindTextureUnit(2, textureIDs[roughnessMap]);
        //glBindTextureUnit(3, textureIDs[roughnessMap]);
        //emissiveFactor
        //glUniform3fv(3, 1, glm::value_ptr(glm::vec3());
       /* if (emissiveFactor.empty()) {
            emissiveFactor = { 0, 0, 0};
        }

        useNormalTexture = 1;
        if (useNormalTexture) {
        }*/

        //glUniform3f(3, static_cast<GLfloat>(emissiveFactor[0]), static_cast<GLfloat>(emissiveFactor[1]), static_cast<GLfloat>(emissiveFactor[2]));
        //glUniform3f(shader->getUniformFromName("emissiveFactor"), static_cast<GLfloat>(emissiveFactor[0]), static_cast<GLfloat>(emissiveFactor[1]), static_cast<GLfloat>(emissiveFactor[2]));
  /*      glUniform1f(shader->getUniformFromName("roughnessFactor"), roughnessFactor);
        glUniform1i(shader->getUniformFromName("useNormalTexture"), useNormalTexture);

        glUniform1i(shader->getUniformFromName("useInstancing"), 0);*/
        
        if (instancing == 1)
        {
            glDrawElements(primitive.mode, indexAccessor.count,
                indexAccessor.componentType,
                BUFFER_OFFSET(indexAccessor.byteOffset));
        }
        else {
            glUniform1i(shader->getUniformFromName("useInstancing"), 1);
            glDrawElementsInstanced(primitive.mode, indexAccessor.count,
                indexAccessor.componentType,
                BUFFER_OFFSET(indexAccessor.byteOffset), instancing);
        }
    }
}

// recursively draw node and children nodes of model
void GLModel::drawModelNodes(tinygltf::Model& model, tinygltf::Node& node, Gloom::Shader* shader) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        drawMesh(model, model.meshes[node.mesh], shader);
    }
    for (size_t i = 0; i < node.children.size(); i++) {
        drawModelNodes(model, model.nodes[node.children[i]], shader);
    }
}

void GLModel::drawModel(Gloom::Shader* shader) {

    //for (size_t i = 0; i < vaos.size(); ++i) {
    //    //auto vao = vaos[i];
    //}
    //GLuint vao = 1;
    //glBindVertexArray(this->vaos[i]);


    
    glBindVertexArray(VAO);
    const tinygltf::Scene& scene = this->scenes[this->defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        drawModelNodes(*this, this->nodes[scene.nodes[i]], shader);
    }
    glBindVertexArray(0);
}


void GLModel::updateInstanceMatrix(std::vector<glm::mat4> newInstanceMatrices) {

    glBindVertexArray(VAO);

 /*   for (size_t i = 0; i < vaos.size(); ++i) {
        
    }*/

    // Shader Storage Buffer Object
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboModelMatrices);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * newInstanceMatrices.size(), newInstanceMatrices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboModelMatrices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindVertexArray(0);
}



