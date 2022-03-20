#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;

out vec4 color;

struct Atlas {    
    sampler2D color;
};  

//in layout(location = 7) uniform Atlas atlas;
layout(binding = 3) uniform sampler2D diffuseTexture;
//uniform Atlas atlas;

void main()
{
    vec3 normal = normalize(normal_in);

//    color = vec4(vec3(1), 1.0);
//    color = vec4(pos, 1.0);
    color = texture(diffuseTexture, textureCoordinates);
//    color = vec4(textureCoordinates, 0, 1);
}