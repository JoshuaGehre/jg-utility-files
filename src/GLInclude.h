#ifndef GLINCLUDE_H_INCLUDED
#define GLINCLUDE_H_INCLUDED

// Based on the operating system the libraries needed
// might slightly vary so just include this file

#if defined(__linux__)

	#define GL_GLEXT_PROTOTYPES
	#include <GLFW/glfw3.h>

#elif defined(__WIN32)

	#include <glad/glad.h>
	#include <GLFW/glfw3.h>

#else

	#error "Can't include OpenGL for this unknown operating system"

#endif

#endif
