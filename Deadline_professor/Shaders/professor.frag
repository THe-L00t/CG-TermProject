#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

// 텍스처 지원
uniform sampler2D uTexture;
uniform bool uUseTexture;

// ⭐⭐⭐ 멀티 라이트 시스템 추가!
struct Light {
    int type;           // 0: DIRECTIONAL, 1: POINT, 2: SPOT
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;

    // 포인트 라이트 감쇠
    float constant;
    float linear;
    float quadratic;

    // 스팟 라이트
    float cutOff;
    float outerCutOff;
};

const int MAX_LIGHTS = 8;
uniform Light lights[MAX_LIGHTS];
uniform int uLightCount;

// 방향성 라이트 계산
vec3 CalculateDirectionalLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    vec3 ambient = light.ambient * objectColor;

    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor;

    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular;

    return (ambient + diffuse + specular) * light.intensity;
}

// 포인트 라이트 계산 (감쇠 포함!)
vec3 CalculatePointLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    vec3 lightDir = normalize(light.position - FragPos);

    vec3 ambient = light.ambient * objectColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor;

    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular;

    // ⭐⭐⭐ 감쇠 계산!
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

// 스팟 라이트 계산
vec3 CalculateSpotLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    vec3 lightDir = normalize(light.position - FragPos);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * objectColor * intensity;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor * intensity;

    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular * intensity;

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

void main()
{
    // 텍스처 적용 (원래 방식대로)
    vec3 objectColor;
    if (uUseTexture) {
        vec4 texColor = texture(uTexture, TexCoord);
        objectColor = texColor.rgb;
    } else {
        objectColor = uColor;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    vec3 result = vec3(0.0);

    // ⭐⭐⭐ 멀티 라이트 처리!
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; i++) {
        if (lights[i].type == 0) {
            result += CalculateDirectionalLight(lights[i], norm, viewDir, objectColor);
        } else if (lights[i].type == 1) {
            result += CalculatePointLight(lights[i], norm, viewDir, objectColor);
        } else if (lights[i].type == 2) {
            result += CalculateSpotLight(lights[i], norm, viewDir, objectColor);
        }
    }

    // 광원이 없으면 레거시 라이트 사용
    if (uLightCount == 0) {
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * uLightColor;

        vec3 lightDir = normalize(uLightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uLightColor;

        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * uLightColor;

        result = (ambient + diffuse + specular) * objectColor;
    }

    FragColor = vec4(result, 1.0);
}