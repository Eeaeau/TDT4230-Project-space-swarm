#include "GLModel.hpp"


//  ------------------------------------------------------ //
//  ------ Code is based on tinygltf loader example ------ //  
//  ------------------------------------------------------ //




void linkAttrib(GLuint bufferID, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLModel::GLModel(const char* filename, unsigned int instancing, std::vector<glm::mat4> instanceMatrix)
{
    loadModel(filename);
    this->instancing = instancing;
    this->instanceMatrix = instanceMatrix;
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
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE,
                    byteStride, BUFFER_OFFSET(accessor.byteOffset));
            }
            else
                std::cout << "vaa missing: " << attrib.first << std::endl;

        }

        if (model.textures.size() > 0) {
            // fixme: Use material's baseColor
            tinygltf::Texture& tex = model.textures[0];

            if (tex.source > -1) {

                GLuint texid;
                glGenTextures(1, &texid);

                tinygltf::Image& image = model.images[tex.source];

                glBindTexture(GL_TEXTURE_2D, texid);
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
                    // ???
                }

                GLenum type = GL_UNSIGNED_BYTE;
                if (image.bits == 8) {
                    // ok
                }
                else if (image.bits == 16) {
                    type = GL_UNSIGNED_SHORT;
                }
                else {
                    // ???
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                    format, type, &image.image.at(0));
            }
        }
    }

    return vbos;
}

void GLModel::bindModelNodes(std::map<int, GLuint> vbos, tinygltf::Model& model,
    tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        bindMesh(vbos, model, model.meshes[node.mesh]);

        if (instancing != 1) {

            // vertex buffer object
            GLuint bufferID;
            glGenBuffers(1, &bufferID);
            glBufferData(GL_ARRAY_BUFFER, instanceMatrix.size() * sizeof(glm::mat4), instanceMatrix.data(), GL_STATIC_DRAW);

            // Can't link to a mat4 so you need to link four vec4s
            linkAttrib(bufferID, 5, 5, GL_FLOAT, sizeof(glm::mat4), (void*)0);
            linkAttrib(bufferID, 6, 5, GL_FLOAT, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
            linkAttrib(bufferID, 7, 5, GL_FLOAT, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
            linkAttrib(bufferID, 8, 5, GL_FLOAT, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));

            // Makes it so the transform is only switched when drawing the next instance
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);
            glVertexAttribDivisor(7, 1);

            glBindVertexArray(0);

        }

    }

    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
        bindModelNodes(vbos, model, model.nodes[node.children[i]]);
    }
}

GLuint GLModel::bindModel() {
    std::map<int, GLuint> vbos;
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const tinygltf::Scene& scene = this->scenes[this->defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < this->nodes.size()));
        bindModelNodes(vbos, *this, this->nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
    // cleanup vbos
    for (size_t i = 0; i < vbos.size(); ++i) {
        glDeleteBuffers(1, &vbos[i]);
    }

    return vao;
}


void GLModel::drawMesh(tinygltf::Model& model, tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        if (instancing == 1)
        {
            glDrawElements(primitive.mode, indexAccessor.count,
                indexAccessor.componentType,
                BUFFER_OFFSET(indexAccessor.byteOffset));
        }
        else {
            glDrawElementsInstanced(primitive.mode, indexAccessor.count,
                indexAccessor.componentType,
                BUFFER_OFFSET(indexAccessor.byteOffset), instancing);
        }
    }
}

// recursively draw node and children nodes of model
void GLModel::drawModelNodes(tinygltf::Model& model, tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        drawMesh(model, model.meshes[node.mesh]);
    }
    for (size_t i = 0; i < node.children.size(); i++) {
        drawModelNodes(model, model.nodes[node.children[i]]);
    }
}

void GLModel::drawModel(GLuint vao) {
    glBindVertexArray(vao);
    const tinygltf::Scene& scene = this->scenes[this->defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        drawModelNodes(*this, this->nodes[scene.nodes[i]]);
    }
    
    glBindVertexArray(0);
}




