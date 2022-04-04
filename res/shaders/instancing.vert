#version 450 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;
in layout(location = 5) mat4 instanceMatrix;

uniform mat4 MVP;
uniform mat4 viewProjectionMatrix;
//uniform mat4 instanceMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;
uniform int useInstance;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 4) vec3 fragPos;
out layout(location = 5) mat3 TBN;
out layout(location = 9) vec3 tangent_out;


void main()
{
    vec3 bitangent = normalize(normalMatrix * bitangent_in);

    tangent_out = normalize( normalMatrix * tangent_in);

	normal_out = normalize(normalMatrix * normal_in);
    
    TBN = mat3(
		 normalize(tangent_in),
		 normalize(bitangent),
		 normalize(normal_out) 
	);

    textureCoordinates_out = textureCoordinates_in;

//    gl_Position = viewProjectionMatrix * instanceMatrix * vec4(position, 1.0f);
    gl_Position = MVP  * vec4(position + vec3(10* mod(gl_InstanceID, 2), 0, 10*gl_InstanceID), 1.0f);

    fragPos = vec3(modelMatrix * vec4(position, 1.0f));
}