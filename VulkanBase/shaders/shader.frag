#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform pc {
    vec4 pcData;
};

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // push constants
    float time = pcData.x;
    vec3 camPos = pcData.yzw;

    // main color
    vec3 col = vec3(0.0, 0.0, 0.0);

    // normals
    vec3 N = normalize(fragNormal);

    // texture
    vec4 albedo = texture(texSampler, fragTexCoord);

    // light
    vec3 lightDir = normalize(vec3(2.0, 3.0, 1.0));
    vec3 lightPos = vec3(0.0, 0.0, 0.5);
    vec3 lightCol = vec3(1.0, 0.95, 0.9);

    // vec3 L = normalize(lightPos - worldPos);  // for point light
    vec3 L = lightDir;

    vec3 V = normalize(camPos - worldPos);
    vec3 H = normalize(L + V);
    
    float atten = 1.0 / dot(lightPos - worldPos, lightPos - worldPos);

    // Blinn-Phong model
    vec3 kd = albedo.rgb;
    vec3 ks = lightCol;
    vec3 ka = vec3(0.17, 0.12, 0.19) * 0.5;
    float q = 500.0;

    col = (kd * max(0.0, dot(N, L) * 0.5 + 0.5) + ks * pow(max(0.0, dot(N, H)), q)) * lightCol;
    col += ka;

    outColor = vec4(col, 1.0);
}