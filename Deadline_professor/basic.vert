#version 330 core

// XMesh format attribute mapping
layout(location = 0) in ivec3 aPositionRaw;  // int16x3, normalized=false -> ivec3
layout(location = 1) in vec3 aNormal;        // int16x3, normalized=true -> vec3 [-1,1]
layout(location = 3) in vec2 aTexCoord;      // half2 -> vec2
layout(location = 5) in vec4 aBoneWeights;   // u8x4, normalized=true -> vec4 [0,1]
layout(location = 6) in ivec4 aBoneIndices;  // u8x4, normalized=false -> ivec4

// Transform matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Skeletal animation (max 100 bones)
uniform mat4 uBones[100];
uniform int uBoneCount;

// Position quantization metadata (XMesh spec)
// Formula: float_position = (int16_raw - offset) * scale
uniform vec3 uPosOffset;   // quantization center
uniform float uPosScale;   // quantization scale

// Output to fragment shader
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main()
{
    // Step 1: Position quantization decoding
    // int16 raw -> float world space
    vec3 decodedPosition = (vec3(aPositionRaw) - uPosOffset) * uPosScale;

    // Step 2: Skeletal animation (bone skinning)
    vec4 pos = vec4(decodedPosition, 1.0);
    vec3 nrm = normalize(aNormal);  // Normal is already normalized to [-1,1]

    if (uBoneCount > 0) {
        // Calculate skinning matrix
        mat4 skinMat = mat4(0.0);
        skinMat += uBones[aBoneIndices.x] * aBoneWeights.x;
        skinMat += uBones[aBoneIndices.y] * aBoneWeights.y;
        skinMat += uBones[aBoneIndices.z] * aBoneWeights.z;
        skinMat += uBones[aBoneIndices.w] * aBoneWeights.w;

        // Apply skinning to position and normal
        pos = skinMat * pos;
        nrm = normalize(mat3(skinMat) * nrm);
    }

    // Step 3: World / View / Projection transform
    vec4 worldPos = uModel * pos;
    FragPos = worldPos.xyz;

    // Step 4: Normal transform (using normal matrix)
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    Normal = normalize(normalMatrix * nrm);

    // Step 5: Texture coordinates (pass through)
    TexCoord = aTexCoord;

    // Step 6: Final position
    gl_Position = uProjection * uView * worldPos;
}
