#version 330 core

// XMESH 스펙에 맞춘 attribute location
layout(location = 0) in vec3 aPosition;  // stream_id 0 (int16x3, normalized=false)
layout(location = 1) in vec3 aNormal;    // stream_id 1 (int16x3, normalized=true -> 이미 [-1,1])
layout(location = 3) in vec2 aTexCoord;  // stream_id 2 (half2)

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

void main()
{
    // Quantized int16 position 디코딩
    // 실제 정점 값: v0: (11784, -21339, -20070)
    // 모델 중심 근사치: (0, -20000, -20000)
    vec3 center = vec3(0.0, -20000.0, -20000.0);

    // 중심을 원점으로 이동 후 스케일
    vec3 decodedPosition = (aPosition - center) * (1.0 / 100.0);

    // Transform
    vec4 worldPos = uModel * vec4(decodedPosition, 1.0);
    FragPos = worldPos.xyz;

    // Normal은 이미 normalized되어 [-1,1] 범위
    Normal = mat3(transpose(inverse(uModel))) * aNormal;

    TexCoord = aTexCoord;

    gl_Position = uProjection * uView * worldPos;
}
