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

	void setUniform(const std::string_view&, int);
	void setUniform(const std::string_view&, float);
	void setUniform(const std::string_view&, float, float);
	void setUniform(const std::string_view&, float, float, float);
	void setUniform(const std::string_view&, float, float, float, float);
	void setUniform(const std::string_view&, glm::vec2&);
	void setUniform(const std::string_view&, glm::vec3&);
	void setUniform(const std::string_view&, glm::vec4&);
	void setUniform(const std::string_view&, glm::mat4&);

private:
	GLuint program{};

	void deleteShader();
	
};

