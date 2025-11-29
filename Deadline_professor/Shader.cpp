#include "Shader.h"

Shader::~Shader()
{
	deleteShader();
}


Shader::Shader(Shader&& other) noexcept
	: program{other.program}
{
	other.program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this not_eq &other) {
		deleteShader();
		program = other.program;
		other.program = 0;
	}
	return *this;
}

std::optional<std::string> Shader::LoadFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::in);
	if (not file) {
		std::cerr << "f-LoadShader failed : " << path << std::endl;
		return std::nullopt;
	}

	std::ostringstream buffer;


	buffer << file.rdbuf();

	return buffer.str();
}

GLuint Shader::AddShader(const std::string_view& src, GLenum type)
{
	GLuint shaderObj = glCreateShader(type);
	
	const GLchar* s = src.data();
	const GLint length = static_cast<GLint>(src.size());

	glShaderSource(shaderObj, 1, &s, &length);
	glCompileShader(shaderObj);

	GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);

	if (not success) {
		GLint logLength;
		glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &logLength);

		// 로그 크기만큼 벡터 할당 (정확한 크기)
		std::vector<GLchar> infoLog(logLength);
		glGetShaderInfoLog(shaderObj, logLength, nullptr, infoLog.data());

		std::cerr << "f-AddShader failed : (" << type << "):\\n"
			<< infoLog.data() << std::endl;

		glDeleteShader(shaderObj);
		return 0;
	}

	return shaderObj;
}


bool Shader::CompileShader(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
{
	if (!std::filesystem::exists(vsPath)) {
		std::cerr << "Vertex shader file not found: " << vsPath << std::endl;
		return false;
	}

	if (!std::filesystem::exists(fsPath)) {
		std::cerr << "Fragment shader file not found: " << fsPath << std::endl;
		return false;
	}

	auto vsSource = LoadFile(vsPath);
	auto fsSource = LoadFile(fsPath);

	if (not vsSource || not fsSource) {
		std::cerr << "f-CompileShader(1) Failed" << std::endl;
		return false;
	}

	std::cout << "  Vertex shader: " << vsSource->size() << " bytes" << std::endl;
	std::cout << "  Fragment shader: " << fsSource->size() << " bytes" << std::endl;

	GLuint vs = AddShader(vsSource.value(), GL_VERTEX_SHADER);
	GLuint fs = AddShader(fsSource.value(), GL_FRAGMENT_SHADER);
	if (not vs || not fs) {
		std::cerr << "f-CompileShader(2) Failed" << std::endl;
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<GLchar> infoLog(logLength);
		glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());

		std::cerr << "f-CompileShader(3) Failed:\\n" << infoLog.data() << std::endl;

		glDeleteProgram(program);
		program = 0;
		return false;
	}

	return true;
}

void Shader::Use() const
{
	if (program not_eq 0) {
		glUseProgram(program);
	}
}

void Shader::Unuse() const
{
	glUseProgram(0);
}

void Shader::setUniform(const std::string_view& uniformName, int value)
{
	std::string name(uniformName);
	glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void Shader::setUniform(const std::string_view& uniformName, float value)
{
	std::string name(uniformName);
	glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void Shader::setUniform(const std::string_view& uniformName, float v1, float v2)
{
	std::string name(uniformName);
	glUniform2f(glGetUniformLocation(program, name.c_str()), v1, v2);
}

void Shader::setUniform(const std::string_view& uniformName, float v1, float v2, float v3)
{
	std::string name(uniformName);
	glUniform3f(glGetUniformLocation(program, name.c_str()), v1, v2, v3);
}

void Shader::setUniform(const std::string_view& uniformName, float v1, float v2, float v3, float v4)
{
	std::string name(uniformName);
	glUniform4f(glGetUniformLocation(program, name.c_str()), v1, v2, v3, v4);
}

void Shader::setUniform(const std::string_view& uniformName, const glm::vec2& value)
{
	std::string name(uniformName);
	glUniform2f(glGetUniformLocation(program, name.c_str()), value.x, value.y);
}

void Shader::setUniform(const std::string_view& uniformName, const glm::vec3& value)
{
	std::string name(uniformName);
	glUniform3f(glGetUniformLocation(program, name.c_str()), value.x, value.y, value.z);
}

void Shader::setUniform(const std::string_view& uniformName, const glm::vec4& value)
{
	std::string name(uniformName);
	glUniform4f(glGetUniformLocation(program, name.c_str()), value.x, value.y, value.z, value.w );
}

void Shader::setUniform(const std::string_view& uniformName, const glm::mat4& value)
{
	std::string name(uniformName);
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

int Shader::getAttrib(const std::string_view& attribName)
{
	std::string name(attribName);
	return glGetAttribLocation(program, name.c_str());
}

void Shader::deleteShader()
{
	if (program not_eq 0) {
		glDeleteProgram(program);
		program = 0;
	}
}

