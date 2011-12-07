#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif


#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>

/**
	Shader is a class designed to allow us to load and use a GLSL shader program in
	our OpenGL application. It allows us to provide filenames for the vertex and 
	fragment shaders, and then creates the shader. It also lets us bind and 
	unbind the GLSL shader program as required.
*/
class Shader {
public:
	Shader(); // Default constructor
	Shader(const char *vsFile, const char *fsFile); // Constructor for creating a shader from two shader filenames
	~Shader(); // Deconstructor for cleaning up
	
	void init(const char *vsFile, const char *fsFile); // Initialize our shader file if we have to
	
	void bind(); // Bind our GLSL shader program
	void unbind(); // Unbind our GLSL shader program
	
	unsigned int id(); // Get the identifier for our program
	GLint getUniformLocation(const char *name) {
		return glGetUniformLocation(shader_id,name);
	}
	void uniformMatrix4fv(const char *name, const GLfloat * value, GLsizei count=1, GLboolean transpose=GL_FALSE) {
		glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
	}
	void uniform1f(const char *name, GLfloat value) {
		glUniform1f(getUniformLocation(name),value);
	}
	void uniform4fv(const char *name, const GLfloat *value, GLsizei count=1) {
		glUniform4fv(getUniformLocation(name), count, value);
	}
	void uniform1i(const char *name, GLint value) {
		glUniform1i(getUniformLocation(name), value);
	}
private:
	unsigned int shader_id; // The shader program identifier
	unsigned int shader_vp; // The vertex shader identifier
	unsigned int shader_fp; // The fragment shader identifier
	
	bool inited; // Whether or not we have initialized the shader
};
