#pragma once
class ResourceManager
{
public:

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;


	void Active();
	void Deactive();

private:
	static ResourceManager* onceInstance;

};

