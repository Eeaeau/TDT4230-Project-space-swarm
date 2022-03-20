#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utilities/glutils.h>

#include <stack>
#include <vector>
#include <cstdio>
#include <stdbool.h>
#include <cstdlib> 
#include <ctime> 
#include <chrono>
#include <fstream>

enum SceneNodeType {
	GEOMETRY, TEXTURED_GEOMETRY, OVERLAY, SPRITE, POINT_LIGHT, SPOT_LIGHT
};

struct SceneNode {
	SceneNode() {
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);

        referencePoint = glm::vec3(0, 0, 0);

        vertexArrayObjectID = -1;
		diffuseTextureID = -1;
		normalTextureID = -1;

        VAOIndexCount = 0;

        nodeType = GEOMETRY;

		lightColor = glm::vec3(4);
		constant = 1.0;
		linear = 0.009;
		quadratic = 0.0032;


		//diffuseTexture = loadPNGFile("../res/textures/normal-map-debug.png");
		//normalTexture = loadPNGFile("../res/textures/normal-map-debug.png");
	}

	glm::vec3 lightColor;
	float constant;
	float linear;
	float quadratic;

	// A list of all children that belong to this node.
	// For instance, in case of the scene graph of a human body shown in the assignment text, the "Upper Torso" node would contain the "Left Arm", "Right Arm", "Head" and "Lower Torso" nodes in its list of children.
	std::vector<SceneNode*> children;
	
	// The node's position and rotation relative to its parent
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	PNGImage diffuseTexture;
	PNGImage normalTexture;
	PNGImage roughnessTexture;

	// The Model matrix for preserving angles is needed
	glm::mat4 modelMatrix;
	glm::mat4 modelViewMatrix;

	// The View matrix
	//glm::mat4 viewMatrix;

	// A MVP transformation matrix representing the transformation of the node's location relative to its parent. This matrix is updated every frame.
	glm::mat4 currentTransformationMatrix;

	// The location of the node's reference point
	glm::vec3 referencePoint;

	// The ID of the VAO containing the "appearance" of this SceneNode.
	int vertexArrayObjectID;
	unsigned int VAOIndexCount;
	unsigned int diffuseTextureID;
	unsigned int normalTextureID;
	unsigned int roughnessTextureID;

	// The ID used for of lights 
	unsigned lightID;

	// Node type is used to determine how to handle the contents of a node
	SceneNodeType nodeType;
};


//struct LightNode: SceneNode {
//	LightNode() {
//		nodeType = POINT_LIGHT;
//	}
//	
//};

SceneNode* createSceneNode();
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
int totalChildren(SceneNode* parent);

// For more details, see SceneGraph.cpp.