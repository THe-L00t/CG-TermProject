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

	std::optional<std::string> LoadFile(const std::filesystem::path&);
	GLuint AddShader(const std::string_view&, GLenum);
	bool CompileShader(const std::filesystem::path&, const std::filesystem::path&);

	void Use() const;
	void Unuse() const;

private:
	GLuint program{};

	void deleteShader();
	
};

