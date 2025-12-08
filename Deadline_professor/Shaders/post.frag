#version 330 core

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D u_SceneTex;

// 색수차 강도
uniform float u_Chromatic = 0.002;

// 비네팅 강도
uniform float u_Vignette = 0.75;

void main()
{
    // --- Chromatic Aberration ---
    float offset = u_Chromatic;

    float r = texture(u_SceneTex, vUV + vec2( offset, 0.0)).r;
    float g = texture(u_SceneTex, vUV).g;
    float b = texture(u_SceneTex, vUV + vec2(-offset, 0.0)).b;

    vec3 color = vec3(r, g, b);

    // --- Vignette ---
    float dist = distance(vUV, vec2(0.5));
    float vignetteMask = smoothstep(u_Vignette, 0.9, dist);
    color *= (1.0 - vignetteMask);

    FragColor = vec4(color, 1.0);
}