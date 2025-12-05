#include "Object.h"

Object::Object()
{
}

Object::~Object()
{
}

void Object::Update(float deltaTime)
{
}

void Object::UpdateModelMat()
{
	modelMat = glm::mat4(1.f);
	modelMat = glm::translate(modelMat, position);
	modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
	modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
	modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
	modelMat = glm::scale(modelMat, scale);
	needsUpdate = false;
}

void Object::SetPosition(const glm::vec3& pos)
{
	position = pos;
	needsUpdate = true;
}

void Object::SetRotation(const glm::vec3& rot)
{
	rotation = rot;
	needsUpdate = true;
}

void Object::SetScale(const glm::vec3& scl)
{
	scale = scl;
	needsUpdate = true;
}

glm::vec3 Object::GetPosition() const
{
	return position;
}

glm::vec3 Object::GetRotation() const
{
	return rotation;
}

glm::vec3 Object::GetScale() const
{
	return scale;
}

glm::mat4 Object::GetModelMat()
{
	if (needsUpdate)
	{
		UpdateModelMat();
	}
	return modelMat;
}

void Object::SetResourceID(const std::string& id)
{
	resourceID = id;
}

std::string Object::GetResourceID() const
{
	return resourceID;
}

void Object::SetActive(bool active)
{
	isActive = active;
}

bool Object::IsActive() const
{
	return isActive;
}

void Object::SetColor(const glm::vec3& col)
{
	color = col;
}

glm::vec3 Object::GetColor() const
{
	return color;
}

