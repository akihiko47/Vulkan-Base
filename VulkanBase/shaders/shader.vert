#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 viewMat;
    mat4 projMat;
};

layout(push_constant) uniform pc {
    mat4 modelMat;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 worldPos;

void main() {
    // interpolation
    fragNormal = (transpose(inverse(modelMat)) * vec4(inNormal, 0.0)).xyz;
    fragTexCoord = inTexCoord;

    // space transformations
    worldPos = (modelMat * vec4(inPosition, 1.0)).xyz;
    gl_Position = projMat * viewMat * modelMat * vec4(inPosition, 1.0);
}