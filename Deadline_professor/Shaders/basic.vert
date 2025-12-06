#version 330 core

// Vertex attributes for OBJ models
layout(location = 0) in vec3 aPosition;   // Position
layout(location = 1) in vec2 aTexCoord;   // Texture coordinates
layout(location = 2) in vec3 aNormal;     // Normal

// Transform matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Texture tiling
uniform vec2 uTextureTiling;

// Output to fragment shader
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main()
{
    // World space position
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;

    // Normal transform (using normal matrix)
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    Normal = normalize(normalMatrix * aNormal);

    // Texture coordinates (apply tiling)
    TexCoord = aTexCoord * uTextureTiling;

    // Final position
    gl_Position = uProjection * uView * worldPos;
}
