#version 450 core

//in layout(location = 0) vec3 normal_in;
//in layout(location = 1) vec2 textureCoordinates;
//in layout(location = 4) vec3 fragPos;
//in layout(location = 5) mat3 TBN;
//in layout(location = 9) vec3 tangent_in;

in vec3 normal;
in vec2 textureCoordinates;
in vec3 fragPos;
in mat3 TBN;
in vec3 tangent;

#define NR_POINT_LIGHTS 4

struct PointLight {    
    vec3 position;

    vec3 lightColor;

    float constant;
    float linear;
    float quadratic;
};  

struct Texture {
    sampler2D diffuse;
    sampler2D normal;
};  

//uniform Texture texture_in;

//in layout(location = 6) uniform Texture texture_in;

layout(binding = 0) uniform sampler2D diffuseTexture; // was not happy using implicit uniform
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D roughnessTexture;

uniform vec3 viewPos;
uniform vec3 ballPos;
uniform vec3 lightTest;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform mat3 normalMatrix;
uniform int useTexture;


layout (location = 0) out vec4 fragColor;  
layout (location = 1) out vec4 brightColor;  

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 CalcReject(vec3 from, vec3 onto) {
    return from - onto*dot(from, onto)/dot(onto, onto);
}

vec3 currentLightPos = vec3(0, 0, 0);
vec3 result = vec3(0);

float ambientStrenght = 0.0;
vec3 ambientColor = vec3(1, 1, 1);

vec3 diffuseColor = vec3(1);
vec3 emissionColor = vec3(0.0);

float specularStrength = 0.15;

vec3 lightColor = vec3(4);

//float constant = 0.1;
//float linear = 0.009;
float quadratic = 0.0032; // for some reason uniform did not work for this one


float ballBaseRadius = 1;
float ballSoftRadius = ballBaseRadius*2;

float emissiveFactor = 0;

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, bool Blinn) {
    
        vec3 lightVec = pointLight.position - fragPos; 
        vec3 lightDir = normalize(lightVec); 

        float diff;

        vec3 reflectDir;
        float spec;

        if (Blinn) {
            vec3 dirToLight = normalize(-lightDir);

            // diffuse shading
            diff = max(dot(normal, dirToLight), 0.0);

             // specular shading
            //        vec3 reflectDir = reflect(-lightDir, normal);
            vec3 halfwayDir = normalize(dirToLight + viewDir);
            float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
            //        float spec = pow(max(dot(halfwayDir,normal ), 0.0), 32);
        }

        else {

            // diffuse shading
            diff = max(dot(normal, lightDir), 0.0);

            reflectDir = reflect(-lightDir, normal);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        }

        // attuantion
        float lightDist = distance(lightDir, fragPos);
        float attenuation =  1.0/ (pointLight.constant + pointLight.linear * lightDist + quadratic * pow(lightDist, 2));   
//        float attenuation =  1.0/ (pointLight.constant + pointLight.linear * lightDist + quadratic * lightDist*lightDist);   

        // shadow 
        vec3 ballVec = ballPos - fragPos;
//        vec3 ballDir = normalize(ballVec);
        
        float shadeFactor = 1.0;
        float shadeSoftFactor = 0.0;

        if (length(ballVec)<length(lightVec) && dot(lightVec, ballVec) >= 0) { // branching not optimal for performance
            vec3 reject = CalcReject(ballVec, lightVec);
//            vec3 rejectDir = normalize(reject);
            
            shadeSoftFactor = max(length(reject)-ballSoftRadius, 0);
            shadeSoftFactor = min(shadeSoftFactor, 1);
            
            shadeFactor = max(length(reject)-ballBaseRadius, 0);
            shadeFactor = min(shadeFactor, 1);

            float mixFactor = abs(length(reject)-ballSoftRadius);
            mixFactor = 1/(1+ 1*mixFactor*mixFactor);

            shadeFactor *= mix(shadeFactor, shadeSoftFactor, mixFactor);

        }

        
        vec3 diffuse = pointLight.lightColor  * diff * diffuseColor * shadeFactor;
        vec3 specular = pointLight.lightColor * spec * specularStrength * shadeFactor; 

        return (diffuse + specular)*attenuation;

}


float fragBrightness(vec4 fragColor) {
	float average = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    return average;
}

void main()
{
    vec3 normal = normalize(normal);

    vec3 viewDir = normalize(viewPos - fragPos);

    if (useTexture == 1) {
            normal = texture(normalTexture, textureCoordinates).rgb * 2 - 1;
            diffuseColor = texture(diffuseTexture, textureCoordinates).rgb;
            normal = normalize(TBN * normal);
            specularStrength = specularStrength/(pow(texture(roughnessTexture, textureCoordinates).r, 2));
//            color = vec4(normal, 1.0);
    }

    for	(int i = 0; i < NR_POINT_LIGHTS; i++) {

        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir, false);

    }

    vec3 ambient = ambientStrenght * ambientColor;

    result += ambient;
    result += emissionColor; 
//    result += dither(textureCoordinates);
    
    
    fragColor = vec4(result, 1.0);

    if (useTexture == 1) {
        fragColor = vec4(vec3(1,0,0), 1.0);
    }
//    brightColor = vec4(1-result, 1.0);
//    FragColor = texture(diffuseTexture, textureCoordinates);
    brightColor = vec4(vec3(0), 1);
    if (fragBrightness(fragColor) > 0.9f) {
		brightColor.rgb += fragColor.rgb;
	}
    brightColor *= emissiveFactor;
}