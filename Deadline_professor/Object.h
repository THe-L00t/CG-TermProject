#pragma once
#include "TotalHeader.h"

class Object
{
public:
	Object();
	virtual ~Object();

	virtual void Update(float);

	void SetPosition(const glm::vec3&);
	void SetRotation(const glm::vec3&);
	void SetScale(const glm::vec3&);

	glm::vec3 GetPosition() const;
	glm::vec3 GetRotation() const;
	glm::vec3 GetScale() const;
	glm::mat4 GetModelMat();

	void SetResourceID(const std::string&);
	std::string GetResourceID() const;

	void SetActive(bool);
	bool IsActive() const;

	void SetColor(const glm::vec3&);
	glm::vec3 GetColor() const;

	void SetTextureID(const std::string&);
	std::string GetTextureID() const;

	void SetTextureTiling(const glm::vec2&);
	glm::vec2 GetTextureTiling() const;

	// Frustum Culling용 바운딩 박스
	virtual void GetBoundingBox(glm::vec3& outMin, glm::vec3& outMax) const;

protected:
	void UpdateModelMat();

	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1.f, 1.f, 1.f};
	glm::mat4 modelMat{1.f};

	std::string resourceID{};
	std::string textureID{};

	bool isActive{true};
	bool needsUpdate{true};

	glm::vec3 color{1.0f, 1.0f, 1.0f}; // 기본값: 흰색
	glm::vec2 textureTiling{1.0f, 1.0f}; // 기본값: 타일링 없음 (1x1)
};

