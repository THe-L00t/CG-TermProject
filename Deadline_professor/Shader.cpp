#include "Shader.h"

Shader::~Shader()
{
}

Shader::Shader(Shader&& other) noexcept
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	// TODO: 여기에 return 문을 삽입합니다.
}

std::optional<std::string> Shader::LoadShader(const std::filesystem::path&)
{
	return std::optional<std::string>();
}
