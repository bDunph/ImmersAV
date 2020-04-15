#ifndef LOG_HPP
#define LOG_HPP

#define GL_LOG_FILE "gl.log"

bool restart_gl_log();
bool gl_log_err(const char* message, ...);
void glfw_error_callback(int error, const char* description);
void log_gl_params();
void print_shader_info_log(GLuint shader_index);
void print_program_info_log(GLuint program);
const char* GL_type_to_string(GLenum type);
void print_all(GLuint program);

#endif
