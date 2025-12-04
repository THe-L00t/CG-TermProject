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

protected:
	void UpdateModelMat();

	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1.f, 1.f, 1.f};
	glm::mat4 modelMat{1.f};

	std::string resourceID{};

	bool isActive{true};
	bool needsUpdate{true};
};

