#include "Light.h"

Light::Light(LightType type)
	: type(type), enabled(true),
	  ambient(0.02f, 0.02f, 0.02f),
	  diffuse(1.0f, 1.0f, 1.0f),
	  specular(1.0f, 1.0f, 1.0f),
	  direction(0.0f, -1.0f, 0.0f),
	  constant(1.0f),
	  linear(0.09f),
	  quadratic(0.032f),
	  cutOff(glm::cos(glm::radians(12.5f))),
	  outerCutOff(glm::cos(glm::radians(17.5f))),
	  intensity(1.0f)
{
}

Light::~Light()
{
}

void Light::SetAmbient(const glm::vec3& amb)
{
	ambient = amb;
}

void Light::SetDiffuse(const glm::vec3& diff)
{
	diffuse = diff;
}

void Light::SetSpecular(const glm::vec3& spec)
{
	specular = spec;
}

glm::vec3 Light::GetAmbient() const
{
	return ambient;
}

glm::vec3 Light::GetDiffuse() const
{
	return diffuse;
}

glm::vec3 Light::GetSpecular() const
{
	return specular;
}

void Light::SetDirection(const glm::vec3& dir)
{
	direction = glm::normalize(dir);
}

glm::vec3 Light::GetDirection() const
{
	return direction;
}

void Light::SetAttenuation(float c, float l, float q)
{
	constant = c;
	linear = l;
	quadratic = q;
}

float Light::GetConstant() const
{
	return constant;
}

float Light::GetLinear() const
{
	return linear;
}

float Light::GetQuadratic() const
{
	return quadratic;
}

void Light::SetCutOff(float innerAngle, float outerAngle)
{
	cutOff = glm::cos(glm::radians(innerAngle));
	outerCutOff = glm::cos(glm::radians(outerAngle));
}

float Light::GetCutOff() const
{
	return cutOff;
}

float Light::GetOuterCutOff() const
{
	return outerCutOff;
}

void Light::SetIntensity(float inten)
{
	intensity = inten;
}

float Light::GetIntensity() const
{
	return intensity;
}

void Light::SetEnabled(bool enable)
{
	enabled = enable;
}

bool Light::IsEnabled() const
{
	return enabled;
}

LightType Light::GetType() const
{
	return type;
}

void Light::ApplyToShader(GLuint shaderProgram, int lightIndex) const
{
	if (!enabled)
		return;

	std::string indexStr = std::to_string(lightIndex);

	GLint ambientLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].ambient").c_str());
	GLint diffuseLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].diffuse").c_str());
	GLint specularLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].specular").c_str());
	GLint intensityLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].intensity").c_str());

	glUniform3fv(ambientLoc, 1, glm::value_ptr(ambient));
	glUniform3fv(diffuseLoc, 1, glm::value_ptr(diffuse));
	glUniform3fv(specularLoc, 1, glm::value_ptr(specular));
	glUniform1f(intensityLoc, intensity);

	switch (type)
	{
	case LightType::DIRECTIONAL:
	{
		GLint typeLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].type").c_str());
		GLint directionLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].direction").c_str());

		glUniform1i(typeLoc, 0);
		glUniform3fv(directionLoc, 1, glm::value_ptr(direction));
		break;
	}
	case LightType::POINT:
	{
		GLint typeLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].type").c_str());
		GLint positionLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].position").c_str());
		GLint constantLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].constant").c_str());
		GLint linearLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].linear").c_str());
		GLint quadraticLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].quadratic").c_str());

		glUniform1i(typeLoc, 1);
		glUniform3fv(positionLoc, 1, glm::value_ptr(position));
		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		break;
	}
	case LightType::SPOT:
	{
		GLint typeLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].type").c_str());
		GLint positionLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].position").c_str());
		GLint directionLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].direction").c_str());
		GLint constantLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].constant").c_str());
		GLint linearLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].linear").c_str());
		GLint quadraticLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].quadratic").c_str());
		GLint cutOffLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].cutOff").c_str());
		GLint outerCutOffLoc = glGetUniformLocation(shaderProgram, ("lights[" + indexStr + "].outerCutOff").c_str());

		glUniform1i(typeLoc, 2);
		glUniform3fv(positionLoc, 1, glm::value_ptr(position));
		glUniform3fv(directionLoc, 1, glm::value_ptr(direction));
		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(cutOffLoc, cutOff);
		glUniform1f(outerCutOffLoc, outerCutOff);
		break;
	}
	}
}

void Light::SetFlickerPattern(FlickerPattern pattern)
{
	flickerPattern = pattern;
	baseIntensity = intensity;  // 현재 밝기를 기본값으로 저장
	flickerTimer = 0.0f;
	flickerOn = true;

	// 패턴에 따라 초기 간격 설정
	switch (pattern)
	{
	case FlickerPattern::SLOW:
		flickerInterval = 0.5f + (rand() % 500) / 1000.0f;  // 0.5~1.0초
		break;
	case FlickerPattern::FAST:
		flickerInterval = 0.1f + (rand() % 200) / 1000.0f;  // 0.1~0.3초
		break;
	case FlickerPattern::RANDOM:
		flickerInterval = (rand() % 1000) / 1000.0f;  // 0~1초
		break;
	case FlickerPattern::DYING:
		flickerInterval = 0.2f + (rand() % 300) / 1000.0f;  // 0.2~0.5초
		break;
	default:
		flickerInterval = 0.0f;
		break;
	}
}

FlickerPattern Light::GetFlickerPattern() const
{
	return flickerPattern;
}

void Light::UpdateFlicker(float deltaTime)
{
	if (flickerPattern == FlickerPattern::NONE) {
		return;
	}

	flickerTimer += deltaTime;

	if (flickerTimer >= flickerInterval)
	{
		flickerTimer = 0.0f;

		switch (flickerPattern)
		{
		case FlickerPattern::SLOW:
		{
			// 느린 깜빡임: 완전히 켜짐/꺼짐
			flickerOn = !flickerOn;
			intensity = flickerOn ? baseIntensity : 0.0f;
			flickerInterval = 0.5f + (rand() % 500) / 1000.0f;
			break;
		}
		case FlickerPattern::FAST:
		{
			// 빠른 깜빡임: 50% 확률로 켜짐/꺼짐
			flickerOn = (rand() % 2 == 0);
			intensity = flickerOn ? baseIntensity : 0.0f;
			flickerInterval = 0.1f + (rand() % 200) / 1000.0f;
			break;
		}
		case FlickerPattern::RANDOM:
		{
			// 랜덤 깜빡임: 밝기도 랜덤하게
			float randomBrightness = (rand() % 100) / 100.0f;  // 0.0~1.0
			intensity = baseIntensity * randomBrightness;
			flickerInterval = (rand() % 1000) / 1000.0f;
			break;
		}
		case FlickerPattern::DYING:
		{
			// 꺼져가는 전구: 점점 어두워짐
			intensity = baseIntensity * (0.1f + (rand() % 40) / 100.0f);  // 10~50% 밝기
			flickerInterval = 0.2f + (rand() % 300) / 1000.0f;
			break;
		}
		default:
			break;
		}
	}
}
