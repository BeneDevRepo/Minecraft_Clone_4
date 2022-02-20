#include "ShaderProgram.h"

#include <cstdio>

#include <vector>

static constexpr auto handleCompileErrors = [](const GLuint shader, const char *const shaderType){
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		printf("Error compiling <%s> shader:\n%s\n\n", shaderType, errorLog.data());

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		exit(1);
	}
};

static constexpr auto handleLinkErrors = [](const GLuint program, const char *const programType) {
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
		printf("Error linking <%s> Program:\n%s\n\n", programType, errorLog.data());
		
		// We don't need the program anymore.
		glDeleteProgram(program);

		// Don't leak shaders either.
		// glDeleteShader(vertexShader);
		// glDeleteShader(fragmentShader);

		exit(1);
	}
};


ShaderProgram::ShaderProgram(const char *const computeFileName):
		shaderProgram(glCreateProgram()) {
	
	FILE *const computeFile = fopen(computeFileName, "rb");
	if(!computeFile) {
		printf("Error opening File <%s>\n", computeFileName);
		return;
	}

	fseek(computeFile, 0, SEEK_END);
	const int vertFileLength = ftell(computeFile);
	fseek(computeFile, 0, SEEK_SET);

	char *const computeSource = new char[vertFileLength + 1];
	fread(computeSource, sizeof(char), vertFileLength, computeFile);
	computeSource[vertFileLength] = 0; // Null Terminator

	fclose(computeFile);


	const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeSource, NULL);
	glCompileShader(computeShader);
	handleCompileErrors(computeShader, computeFileName);

	glAttachShader(shaderProgram, computeShader);
	glLinkProgram(shaderProgram);
	handleLinkErrors(shaderProgram, "Main");

	glDetachShader(shaderProgram, computeShader);
	glDeleteShader(computeShader);

	delete[] computeSource;
}

ShaderProgram::ShaderProgram(const char *const vertFileName, const char *const fragFileName):
		shaderProgram(glCreateProgram()) {
	
	FILE *const vertFile = fopen(vertFileName, "rb");
	if(!vertFile) {
		printf("Error opening File <%s>\n", vertFileName);
		return;
	}

	fseek(vertFile, 0, SEEK_END);
	const int vertFileLength = ftell(vertFile);
	fseek(vertFile, 0, SEEK_SET);

	char *const vertSource = new char[vertFileLength + 1];
	fread(vertSource, sizeof(char), vertFileLength, vertFile);
	vertSource[vertFileLength] = 0; // Null Terminator

	fclose(vertFile);



	FILE *const fragFile = fopen(fragFileName, "rb");
	if(!fragFile) {
		printf("Error opening File <%s>\n", fragFileName);
		return;
	}

	fseek(fragFile, 0, SEEK_END);
	const int fragFileLength = ftell(fragFile);
	fseek(fragFile, 0, SEEK_SET);

	char *const fragSource = new char[fragFileLength + 1];
	fread(fragSource, sizeof(char), fragFileLength, fragFile);
	fragSource[fragFileLength] = 0; // Null Terminator

	fclose(fragFile);




	const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertSource, NULL);
	glCompileShader(vertexShader);
	handleCompileErrors(vertexShader, vertFileName);

	const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragSource, NULL);
	glCompileShader(fragmentShader);
	handleCompileErrors(fragmentShader, fragFileName);

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	handleLinkErrors(shaderProgram, "Main");

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	delete[] vertSource;
	delete[] fragSource;
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(shaderProgram);
}