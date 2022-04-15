#version 450 core

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(std430, binding = 0) buffer modelMatrices
{
    mat4 model[];
};

uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform int useInstancing;

out vec3 normal;
out vec3 tangent;
out vec3 position;
out vec2 texcoord;
out mat3 TBN;

void main(){

	vec3 bitangent = normalize(cross(in_normal, in_tangent));

	tangent = normalize(normalMatrix * in_tangent);

	normal = normalize(normalMatrix * in_normal);
    
    TBN = mat3(
		 normalize(tangent),
		 normalize(bitangent),
		 normalize(normal) 
	);

	position = gl_Position.xyz;

	gl_Position = MVP * vec4(in_vertex, 1);

	if (useInstancing == 1){
		gl_Position = MVP * model[gl_InstanceID] * vec4(in_vertex, 1);
	}
//	normal = normalize(mat3(MVP) * in_normal);
//	tangent = normalize(in_tangent);
	position = in_vertex;
	texcoord = in_texcoord;
}