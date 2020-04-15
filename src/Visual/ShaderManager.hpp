#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <string>

std::string readFile(const char* filePath);
bool load_shader(const char* filename, std::string &string);
bool shader_compile_check(GLuint shader);
bool shader_link_check(GLuint program);
bool is_valid(GLuint program);

#endif
