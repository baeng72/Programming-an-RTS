#include "GLERR.h"
#include "../../Core/Log.h"
void printGLError(GLenum err,const char* pfile, int pline) {
	if (err == GL_NO_ERROR)
		return;
	char* errmsg = "";
	switch (err) {
	case	GL_INVALID_ENUM:
		errmsg = "Given when an enumeration parameter is not a legal enumeration for that function.This is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.";
		break;

	case GL_INVALID_VALUE:
		errmsg = "Given when a value parameter is not a legal value for that function.This is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.";
		break;
	case GL_INVALID_OPERATION:
		errmsg = "Given when the set of state for a command is not legal for the parameters given to that command.It is also given for commands where combinations of parameters define what the legal parameters are.";
		break;
	case GL_STACK_OVERFLOW:
		errmsg = "Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size.";
		break;
	case GL_STACK_UNDERFLOW:
		errmsg = "Given when a stack popping operation cannot be done because the stack is already at its lowest point.";
		break;
	case GL_OUT_OF_MEMORY:
		errmsg = "Given when performing an operation that can allocate memory, and the memory cannot be allocated.The results of OpenGL functions that return this error are undefined; it is allowable for partial execution of an operation to happen in this circumstance.";
		break;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		errmsg = "Given when doing anything that would attempt to read from or write / render to a framebuffer that is not complete.";
		break;
	case GL_CONTEXT_LOST:
		errmsg = "Given if the OpenGL context has been lost, due to a graphics card reset.";
		break;

	}
	LOG_ERROR("OpenGL Error: {0} in {1}, {2}", errmsg,pfile,pline);
}