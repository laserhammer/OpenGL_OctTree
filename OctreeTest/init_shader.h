#ifndef INIT_SHADER_H
#define INIT_SHADER_H
#include"utils.h"


static char* textFileRead(char* fn);

GLuint initShaders(char** shaders, GLenum* types, int numShaders);

#endif