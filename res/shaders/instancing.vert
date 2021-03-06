#version 450 core

in layout(location = 0) vec3 position_in;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;
in layout(location = 5) vec3 instancePos;
in layout(location = 6) mat4 instanceMatrix;

layout(std430, binding = 0) buffer modelMatrices
{
    mat4 model[];
};

uniform mat4 MVP;
uniform mat4 viewProjectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
//uniform vec3 viewPos;

out vec3 normal;
out vec2 textureCoordinates;
out vec3 fragPos;
out vec3 tangent;
out mat3 TBN;

void main()
{
    vec3 bitangent = normalize(normalMatrix * bitangent_in);

    tangent = normalize(normalMatrix * tangent_in);

	normal = normalize(normalMatrix * normal_in);
    
    TBN = mat3(
		 normalize(tangent),
		 normalize(bitangent),
		 normalize(normal) 
	);

    textureCoordinates = textureCoordinates_in;

//    vec3 position = position_in + instancePos;
    vec3 position = position_in;

    // calculates current position
//	position = vec3(modelMatrix * instanceMatrix * vec4(position, 1.0f));

    gl_Position = MVP * model[gl_InstanceID] * vec4(position, 1.0f);
//    gl_Position = viewProjectionMatrix * vec4(position, 1.0f);
//    gl_Position = MVP * instanceMatrix * vec4(position, 1.0f);

//    gl_Position = viewProjectionMatrix * instanceMatrix * vec4(position, 1.0f);
//    gl_Position = MVP  * vec4(position + vec3(10* mod(gl_InstanceID, 2), 0, 10*gl_InstanceID), 1.0f);

    fragPos = vec3(modelMatrix * model[gl_InstanceID] * vec4(position, 1.0f));
//    fragPos = vec3(modelMatrix * instanceMatrix* vec4(position, 1.0f));
}