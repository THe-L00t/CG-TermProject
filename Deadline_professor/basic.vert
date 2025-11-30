#version 330 core

// ✅ XMesh 포맷 명세에 따른 정확한 attribute 매핑
layout(location = 0) in ivec3 aPositionRaw;  // int16x3, normalized=false → ivec3
layout(location = 1) in vec3 aNormal;        // int16x3, normalized=true → vec3 [-1,1]
layout(location = 3) in vec2 aTexCoord;      // half2 → vec2
layout(location = 5) in vec4 aBoneWeights;   // u8x4, normalized=true → vec4 [0,1]
layout(location = 6) in ivec4 aBoneIndices;  // u8x4, normalized=false → ivec4

// Transform matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Skeletal animation (최대 100개 본)
uniform mat4 uBones[100];
uniform int uBoneCount;

// ✅ Position quantization 메타데이터 (XMesh 명세)
// 공식: float_position = (int16_raw - offset) * scale
uniform vec3 uPosOffset;   // quantization center
uniform float uPosScale;   // quantization scale

// Output to fragment shader
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main()
{
    // 🔍 TEST 1: 모든 정점을 원점에 단일 점으로 표시 (VAO/렌더링 파이프라인 테스트)
    gl_Position = vec4(0.0, 0.0, -1.0, 1.0);  // NDC 좌표
    FragPos = vec3(0.0);
    Normal = vec3(0.0, 1.0, 0.0);
    TexCoord = vec2(0.0);
    return;

    // ✅ Step 1: Position quantization 디코딩
    // int16 raw → float world space
    vec3 decodedPosition = (vec3(aPositionRaw) - uPosOffset) * uPosScale;

    // 🔍 DEBUG: 디코딩된 위치가 [-1, 1] 범위인지 확인
    // 범위 밖이면 빨간색으로 표시
    // if (abs(decodedPosition.x) > 2.0 || abs(decodedPosition.y) > 2.0 || abs(decodedPosition.z) > 2.0) {
    //     gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    //     FragPos = vec3(0.0);
    //     Normal = vec3(0.0, 1.0, 0.0);
    //     TexCoord = vec2(0.0);
    //     return;
    // }

    // ✅ Step 2: Skeletal animation (bone skinning)
    vec4 pos = vec4(decodedPosition, 1.0);
    vec3 nrm = normalize(aNormal);  // Normal은 이미 [-1,1]로 normalized

    if (uBoneCount > 0) {
        // Skinning matrix 계산
        mat4 skinMat = mat4(0.0);
        skinMat += uBones[aBoneIndices.x] * aBoneWeights.x;
        skinMat += uBones[aBoneIndices.y] * aBoneWeights.y;
        skinMat += uBones[aBoneIndices.z] * aBoneWeights.z;
        skinMat += uBones[aBoneIndices.w] * aBoneWeights.w;

        // Position과 Normal에 스키닝 적용
        pos = skinMat * pos;
        nrm = normalize(mat3(skinMat) * nrm);
    }

    // ✅ Step 3: World / View / Projection 변환
    vec4 worldPos = uModel * pos;
    FragPos = worldPos.xyz;

    // ✅ Step 4: Normal 변환 (normal matrix 사용)
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    Normal = normalize(normalMatrix * nrm);

    // ✅ Step 5: Texture coordinates (그대로 전달)
    TexCoord = aTexCoord;

    // ✅ Step 6: Final position
    gl_Position = uProjection * uView * worldPos;
}
