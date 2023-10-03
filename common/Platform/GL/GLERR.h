#pragma once
#include <glad/glad.h>
void printGLError(GLenum,const char*pfile, int pline);
#ifdef NDEBUG
#define GLERR()
#else
#define GLERR() (printGLError(glGetError(),__FILE__,__LINE__))
#endif