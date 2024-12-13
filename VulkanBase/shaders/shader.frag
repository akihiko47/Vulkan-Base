#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // main color
    vec3 col = vec3(0.0, 0.0, 0.0);

    // normals
    vec3 N = normalize(fragNormal);

    // texture
    vec4 albedo = texture(texSampler, fragTexCoord);

    // light
    vec3 lightPos = vec3(0.0, 0.0, 1.5);
    vec3 lightCol = vec3(1.0, 0.95, 0.9)
    vec3 L = normalize(lightPos - worldPos);
    
    float atten = 1.0 / dot(lightPos - worldPos, lightPos - worldPos);

    outColor = vec4(albedo.rgb * atten, 1.0);
}