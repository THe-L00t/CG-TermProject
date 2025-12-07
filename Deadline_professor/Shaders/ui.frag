#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uTime;

// 깜빡임 속도
const float speed = 3.0;

void main()
{
    // 텍스처 색
    vec4 texColor = texture(uTexture, vUV);

    // GPU에서 시간 기반 깜빡임 계산
    float alpha = 0.5 + 0.5 * sin(uTime * speed);

    // 최종 출력
    FragColor = vec4(texColor.rgb, texColor.a * alpha);
}
