#pragma once

// The one place that knows which GL loader exists. Desktop links GLEW against real OpenGL 3.3
#ifdef __EMSCRIPTEN__
	#include <GLES3/gl3.h>
	#include <emscripten/emscripten.h>
	#include <emscripten/html5.h>
	#define GLSL_HEADER "#version 300 es\nprecision highp float;\n"
#else
	#include <glew.h>
	#define GLSL_HEADER "#version 330 core\n"
#endif

#include <GLFW/glfw3.h>
