#include "shader.h"

#include <fstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexSrc = ReadFile(vertexPath);
	const char* vertexSrcCstr = vertexSrc.c_str();
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSrcCstr, NULL);
	glCompileShader(vertex);
	CheckShaderError(vertex, GL_COMPILE_STATUS);

	std::string fragmentSrc = ReadFile(fragmentPath);
	const char* fragmentSrcCstr = fragmentSrc.c_str();
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSrcCstr, NULL);
	glCompileShader(fragment);
	CheckShaderError(fragment, GL_COMPILE_STATUS);

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	CheckShaderError(program, GL_LINK_STATUS);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

std::string Shader::ReadFile(const char* fileName) {
	std::ifstream file;
	file.open(fileName);

	std::string output;
	std::string line;

	if (file.is_open()) {
		while (file.good()) {
			getline(file, line);
			output.append(line + "\n");
		}
	} else {
		std::cerr << "Unable to load shader: " << fileName << std::endl;
	}

	return output;
}

void Shader::CheckShaderError(GLuint shader, GLuint flag) {
	GLint success = 0;
	GLchar error[1024] = { 0 };
	if (flag == GL_COMPILE_STATUS) {
		glGetShaderiv(shader, flag, &success);
		if (success == GL_FALSE) {
			glGetShaderInfoLog(shader, sizeof(error), NULL, error);
			std::cerr << "Error compiling shader : " << error << "'" << std::endl;
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (success == GL_FALSE) {
			glGetShaderInfoLog(shader, sizeof(error), NULL, error);
			std::cerr << "Error linking shaders : " << error << "'" << std::endl;
		}
	}
}