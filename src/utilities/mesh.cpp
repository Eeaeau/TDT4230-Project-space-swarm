#include "mesh.h"

//bool loadModel(GLModel& model, const char* filename) {
//    tinygltf::TinyGLTF loader;
//    std::string err;
//    std::string warn;
//
//    bool res = loader.LoadASCIIFromFile(&(model), &err, &warn, filename);
//    if (!warn.empty()) {
//        std::cout << "WARN: " << warn << std::endl;
//    }
//
//    if (!err.empty()) {
//        std::cout << "ERR: " << err << std::endl;
//    }
//
//    if (!res)
//        std::cout << "Failed to load glTF: " << filename << std::endl;
//    else
//        std::cout << "Loaded glTF: " << filename << std::endl;
//
//    return res;
//}

//bool loadModel(GLModel& model, const char* filename);

//static std::string GetFilePathExtension(const std::string& FileName) {
//    if (FileName.find_last_of(".") != std::string::npos)
//        return FileName.substr(FileName.find_last_of(".") + 1);
//    return "";
//}