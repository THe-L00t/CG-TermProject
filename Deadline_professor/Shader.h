#pragma once

#include <filesystem>
#include <gl/freeglut.h>

class Shader
{
public:
	Shader() = default;
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	bool LoadShader(const std::filesystem::path&, const std::filesystem::path&);

private:
	GLuint program{};

	void deleteShader();
	std::optional<std::string> LoadFile(const std::filesystem::path&);
};

