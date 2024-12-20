#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 camPos;

layout(location = 0) out vec4 outColor;

void main() {
    // main color
    vec3 col = vec3(0.0, 0.0, 0.0);

    // normals
    vec3 N = normalize(fragNormal);

    // texture
    vec4 albedo = texture(texSampler, fragTexCoord);

    // light
    vec3 lightPos = vec3(0.0, 0.0, 0.5);
    vec3 lightCol = vec3(1.0, 0.9, 0.7) * 0.3;

    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(camPos);
    vec3 H = normalize(L + V);
    
    float atten = 1.0 / dot(lightPos - worldPos, lightPos - worldPos);

    // Blinn-Phong model
    vec3 kd = albedo.rgb;
    vec3 ks = lightCol;
    float q = 500.0;

    col = (kd * max(0.0, dot(N, L) * 0.5 + 0.5) + ks * pow(max(0.0, dot(N, H)), q)) * atten * lightCol;

    outColor = vec4(col, 1.0);
}