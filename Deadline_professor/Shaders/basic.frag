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

// 광원 구조체
struct Light {
    int type;           // 0: DIRECTIONAL, 1: POINT, 2: SPOT
    vec3 position;      // 포인트/스팟 라이트 위치
    vec3 direction;     // 방향성/스팟 라이트 방향
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

// 최대 8개 광원 지원
const int MAX_LIGHTS = 8;
uniform Light lights[MAX_LIGHTS];
uniform int uLightCount;

// 방향성 라이트 계산
vec3 CalculateDirectionalLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    // Ambient
    vec3 ambient = light.ambient * objectColor;

    // Diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor;

    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular;

    return (ambient + diffuse + specular) * light.intensity;
}

// 포인트 라이트 계산
vec3 CalculatePointLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    vec3 lightDir = normalize(light.position - FragPos);
    
    // Ambient
    vec3 ambient = light.ambient * objectColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor;

    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular;

    // 감쇠 계산
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

// 스팟 라이트 계산
vec3 CalculateSpotLight(Light light, vec3 norm, vec3 viewDir, vec3 objectColor)
{
    vec3 lightDir = normalize(light.position - FragPos);
    
    // 스팟 라이트 효과
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 감쇠 계산
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Ambient
    vec3 ambient = light.ambient * objectColor * intensity;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * objectColor * intensity;

    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light.specular * intensity;

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

void main()
{
    // 텍스처 적용 여부에 따라 색상 결정
    vec3 objectColor;
    if (uUseTexture) {
        vec4 texColor = texture(uTexture, TexCoord);
        objectColor = texColor.rgb;

        // 디버깅: UV 좌표가 올바른지 확인
        if (length(objectColor) < 0.01) {
            FragColor = vec4(TexCoord, 0.0, 1.0);
            return;
        }
    } else {
        objectColor = uColor;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    vec3 result = vec3(0.0);

    // 여러 광원 처리
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; i++) {
        if (lights[i].type == 0) {
            // 방향성 라이트
            result += CalculateDirectionalLight(lights[i], norm, viewDir, objectColor);
        } else if (lights[i].type == 1) {
            // 포인트 라이트
            result += CalculatePointLight(lights[i], norm, viewDir, objectColor);
        } else if (lights[i].type == 2) {
            // 스팟 라이트
            result += CalculateSpotLight(lights[i], norm, viewDir, objectColor);
        }
    }

    // 광원이 없으면 기본 라이트 사용 (하위호환성)
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

/*
void main()
{
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * uLightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;

    // 텍스처 적용 여부에 따라 색상 결정
    vec3 objectColor;
    if (uUseTexture) {
        vec4 texColor = texture(uTexture, TexCoord);
        objectColor = texColor.rgb;

        // 디버깅: UV 좌표가 올바른지 확인
        if (length(objectColor) < 0.01) {
            // 텍스처가 검은색이면 UV 좌표를 색상으로 표시
            FragColor = vec4(TexCoord, 0.0, 1.0);
            return;
        }
    } else {
        objectColor = uColor;
    }

    // 라이팅 적용
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
*/
