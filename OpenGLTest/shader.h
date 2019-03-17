#pragma once

#include <string>
#define GLEW_STATIC
#include <GLEW/glew.h>
#include <glm/glm.hpp>

class Shader {
	public:
		Shader(const char* vertexPath, const char* fragmentPath);
		void use() const {
			glUseProgram(program);
		};
		void setBool(const std::string &name, bool value) const {
			glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
		}
		void setInt(const std::string &name, int value) const {
			glUniform1i(glGetUniformLocation(program, name.c_str()), value);
		}
		void setFloat(const std::string &name, float value) const {
			glUniform1f(glGetUniformLocation(program, name.c_str()), value);
		}
		void setVec2(const std::string &name, const glm::vec2 &value, int count = 1) const {
			glUniform2fv(glGetUniformLocation(program, name.c_str()), count, &value[0]);
		}
		void setVec3(const std::string &name, const glm::vec3 &value, int count = 1) const {
			glUniform3fv(glGetUniformLocation(program, name.c_str()), count, &value[0]);
		}
		void setVec4(const std::string &name, const glm::vec4 &value, int count = 1) const {
			glUniform4fv(glGetUniformLocation(program, name.c_str()), count, &value[0]);
		}
		void setMat2(const std::string &name, const glm::mat2 &mat, int count = 1) const {
			glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), count, GL_FALSE, &mat[0][0]);
		}
		void setMat3(const std::string &name, const glm::mat3 &mat, int count = 1) const {
			glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), count, GL_FALSE, &mat[0][0]);
		}
		void setMat4(const std::string &name, const glm::mat4 &mat, int count = 1) const {
			glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), count, GL_FALSE, &mat[0][0]);
		}
	private:
		std::string ReadFile(const char * fileName);
		void CheckShaderError(GLuint shader, GLuint flag);
		GLuint program;
};