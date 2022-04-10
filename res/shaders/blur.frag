#version 450 core

out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D screenTexture;


void main()
{
    vec3 fragment = texture(screenTexture, texCoords).rgb;

    int maxLOD = 8;
    FragColor=  vec4(vec3(0), 1);
      for (int lod = 1; lod< maxLOD; lod++) {
        float x = cosh(0.3*lod);
        FragColor.rgb += 2/x * textureLod(screenTexture, texCoords, lod).rgb;
      }
//

//    FragColor.rgb = fragment;
//    FragColor = textureLod(screenTexture, texCoords, 6);
}