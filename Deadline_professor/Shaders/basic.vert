#version 330 core

// Standard vertex attributes
layout(location = 0) in vec3 aPosition;   // float3 position
layout(location = 1) in vec2 aTexCoord;   // float2 texture coordinates
layout(location = 2) in vec3 aNormal;     // float3 normal
layout(location = 3) in ivec4 aBoneIDs;   // int4 bone indices
layout(location = 4) in vec4 aBoneWeights; // float4 bone weights

// Transform matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Skeletal animation
const int MAX_BONES = 100;
uniform mat4 uBoneTransforms[MAX_BONES];
uniform bool uUseSkinning;

// Output to fragment shader
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main()
{
    vec4 localPosition = vec4(aPosition, 1.0);
    vec3 localNormal = aNormal;

    // Apply skeletal animation if enabled
    if (uUseSkinning) {
        // 본 가중치 합계 계산
        float totalWeight = aBoneWeights[0] + aBoneWeights[1] + aBoneWeights[2] + aBoneWeights[3];

        // 본의 영향을 받는 정점인 경우에만 본 변환 적용
        if (totalWeight > 0.001) {
            mat4 boneTransform = mat4(0.0);

            for (int i = 0; i < 4; ++i) {
                if (aBoneIDs[i] >= 0 && aBoneIDs[i] < MAX_BONES && aBoneWeights[i] > 0.0) {
                    boneTransform += uBoneTransforms[aBoneIDs[i]] * aBoneWeights[i];
                }
            }

            // Apply bone transformation
            localPosition = boneTransform * vec4(aPosition, 1.0);

            // Transform normal
            mat3 boneNormalMatrix = mat3(boneTransform);
            localNormal = boneNormalMatrix * aNormal;
        }
        // 본의 영향을 받지 않는 정점은 원래 위치 유지 (항등 변환)
    }

    // World space position
    vec4 worldPos = uModel * localPosition;
    FragPos = worldPos.xyz;

    // Normal transform (using normal matrix)
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    Normal = normalize(normalMatrix * localNormal);

    // Texture coordinates (pass through)
    TexCoord = aTexCoord;

    // Final position
    gl_Position = uProjection * uView * worldPos;
}
