/*
*	Class init_shader Author: Benjamin Robbins, Jesse Charlan
*	Function: Instantiates array of shader files passed in
*	Dependencies: none
*/

#include "init_shader.h"

//This code courtesy of http://www.lighthouse3d.com/tutorials/glsl-core-tutorial/creating-a-shader/
static char* textFileRead(char* fn)
{
	FILE* fp;
	char* content = NULL;
	errno_t err;

	int count = 0;

	if(fn != NULL)
	{
		err = fopen_s(&fp, fn, "rt"); 

		if(fp != NULL) 
		{
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if(count > 0)
			{
				content = new char[count+1];
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			err = fclose(fp);
		}
	}
	return content;
}

GLuint initShaders(char** shaders, GLenum* types, int numShaders)
{
	GLuint program = glCreateProgram();
	//loop through the shaders passed in and initialize them
	for(int i = 0; i < numShaders; i++)
	{
		//Create the shader
		GLuint shader;

		//get a shader handler
		shader = glCreateShader(types[i]);
		//read the shader from the source file
		const char* shaderSource = textFileRead(shaders[i]);
		//pass source to GL
		glShaderSource(shader, 1, &shaderSource, NULL);
		//delete the memory from the source text
		delete[] shaderSource;
		//Compile shader
		glCompileShader(shader);

		//Check compilation errors
		GLint  compiled;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled ) 
		{
			std::cerr << shaders[i] << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
			char* logMsg = new char[logSize];
			glGetShaderInfoLog( shader, logSize, NULL, logMsg );
			std::cerr << logMsg << std::endl;
			delete [] logMsg;

			exit( EXIT_FAILURE );
		}

		glAttachShader(program, shader);
	}
	//Tell the fragment shader which buffer to write to
	glBindFragDataLocation(program, 0, "outColor");

	glLinkProgram(program);

	//Check linking errors
	GLint  linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) 
	{
		std::cerr << "Shader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog( program, logSize, NULL, logMsg );
		std::cerr << logMsg << std::endl;
		delete [] logMsg;

		exit( EXIT_FAILURE );
    }
	
	return program;
}

