#version 450 core

out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D screenTexture;


void main()
{
    vec3 fragment = texture(screenTexture, texCoords).rgb;

//    FragColor.rgb = fragment;
//    FragColor = textureLod(screenTexture, texCoords, 6);
}