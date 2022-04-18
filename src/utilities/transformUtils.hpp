#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utilities/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace glm {
	template<typename T, qualifier Q>
    GLM_FUNC_QUALIFIER mat<4, 4, T, Q> composeTransform(vec<3, T, Q>& Scale, qua<T, Q>& Orientation, vec<3, T, Q>& Translation, vec<3, T, Q>& Skew, vec<4, T, Q>& Perspective, vec<3, T, Q> ReferencePoint = {0, 0, 0}) {
        mat4 transformationMatrix =
            translate(Translation)
            * translate(ReferencePoint)
            * toMat4(Orientation)
            * scale(Scale)
            * translate(-ReferencePoint);
        return transformationMatrix;
	}	
    
    template<typename T, qualifier Q>
    GLM_FUNC_QUALIFIER mat<4, 4, T, Q> composeTransform(vec<3, T, Q>& Scale, vec<3, T>& Rotation, vec<3, T, Q>& Translation, vec<3, T, Q>& Skew, vec<4, T, Q>& Perspective, vec<3, T, Q> ReferencePoint = {0, 0, 0}) {
        glm::mat4 transformationMatrix =
            translate(Translation)
            * translate(ReferencePoint)
            * rotate(Rotation.y, glm::vec3(0, 1, 0))
            * rotate(Rotation.x, glm::vec3(1, 0, 0))
            * rotate(Rotation.z, glm::vec3(0, 0, 1))
            * scale(Scale)
            * translate(-ReferencePoint);
        return transformationMatrix;
	}
}