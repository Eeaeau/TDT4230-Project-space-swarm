#version 450 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;


uniform mat4 MVP;
uniform mat4 instanceMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;
//uniform int useInstance;

//out layout(location = 0) vec3 normal_out;
//out layout(location = 1) vec2 textureCoordinates_out;
//out layout(location = 4) vec3 fragPos;
//out layout(location = 5) mat3 TBN;
//out layout(location = 9) vec3 tangent_out;

out vec3 normal;
out vec2 textureCoordinates;
out vec3 fragPos;
out vec3 tangent;
out mat3 TBN;


void main()
{
    vec3 bitangent = normalize(normalMatrix * bitangent_in);

    tangent = normalize( normalMatrix * tangent_in);

	normal = normalize(normalMatrix * normal_in);
    
    TBN = mat3(
		 normalize(tangent),
		 normalize(bitangent),
		 normalize(normal) 
	);

    textureCoordinates = textureCoordinates_in;

    gl_Position = MVP * vec4(position, 1.0f);
    fragPos = vec3(modelMatrix * vec4(position, 1.0f));
}