#pragma once
#include "TotalHeader.h"
#include "Object.h"

enum class LightType
{
	DIRECTIONAL,
	POINT,
	SPOT
};

// ⭐⭐⭐ 깜빡임 패턴 종류
enum class FlickerPattern
{
	NONE,           // 깜빡이지 않음
	SLOW,           // 느린 깜빡임 (0.5~1초 간격)
	FAST,           // 빠른 깜빡임 (0.1~0.3초 간격)
	RANDOM,         // 랜덤 깜빡임
	DYING           // 꺼져가는 전구 (점점 어두워짐)
};

class Light : public Object
{
public:
	Light(LightType type = LightType::POINT);
	virtual ~Light();

	void SetAmbient(const glm::vec3&);
	void SetDiffuse(const glm::vec3&);
	void SetSpecular(const glm::vec3&);

	glm::vec3 GetAmbient() const;
	glm::vec3 GetDiffuse() const;
	glm::vec3 GetSpecular() const;

	void SetDirection(const glm::vec3&);
	glm::vec3 GetDirection() const;

	void SetAttenuation(float constant, float linear, float quadratic);
	float GetConstant() const;
	float GetLinear() const;
	float GetQuadratic() const;

	void SetCutOff(float innerAngle, float outerAngle);
	void SetSpotAngle(float innerAngle, float outerAngle) { SetCutOff(innerAngle, outerAngle); }  // SetCutOff의 별칭
	float GetCutOff() const;
	float GetOuterCutOff() const;

	void SetIntensity(float);
	float GetIntensity() const;

	void SetEnabled(bool);
	bool IsEnabled() const;

	LightType GetType() const;

	void ApplyToShader(GLuint shaderProgram, int lightIndex) const;

	// ⭐⭐⭐ 깜빡임 관련 함수
	void SetFlickerPattern(FlickerPattern pattern);
	FlickerPattern GetFlickerPattern() const;
	void UpdateFlicker(float deltaTime);  // 매 프레임마다 호출

private:
	LightType type;
	bool enabled;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 direction;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	float intensity;

	// ⭐⭐⭐ 깜빡임 관련 멤버 변수
	FlickerPattern flickerPattern{ FlickerPattern::NONE };
	float baseIntensity{ 1.0f };           // 원래 밝기
	float flickerTimer{ 0.0f };            // 깜빡임 타이머
	float flickerInterval{ 0.0f };         // 다음 깜빡임까지 시간
	bool flickerOn{ true };                // 현재 켜져있는지 여부

};
