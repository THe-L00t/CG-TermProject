#pragma once
#include "TotalHeader.h"
class Shader
{
public:
	Shader() = default;
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	std::optional<std::string> LoadShader(const std::filesystem::path&);

private:

};

