#pragma once
#include "TotalHeader.h"
#include "Object.h"

enum class LightType
{
	DIRECTIONAL,
	POINT,
	SPOT
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
	float GetCutOff() const;
	float GetOuterCutOff() const;

	void SetIntensity(float);
	float GetIntensity() const;

	void SetEnabled(bool);
	bool IsEnabled() const;

	LightType GetType() const;

	void ApplyToShader(GLuint shaderProgram, int lightIndex) const;

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
};
