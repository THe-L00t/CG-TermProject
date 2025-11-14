#include "Shader.h"
import TotalHeader

Shader::~Shader()
{
	deleteShader();

}


Shader::Shader(Shader&& other) noexcept
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	// TODO: 여기에 return 문을 삽입합니다.
}

std::optional<std::string> Shader::LoadShader(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
{



	return true;
}

void Shader::deleteShader()
{
	if (program != 0) {
		glDeleteProgram(program);
		program = 0;
	}
}

std::optional<std::string> Shader::LoadFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::in);
	if (not file) {
		std::cerr << "f:LoadShader failed : " << path << std::endl;
		return std::nullopt;
	}

	std::ostringstream buffer;


	buffer << file.rdbuf();

	return buffer.str();
}
