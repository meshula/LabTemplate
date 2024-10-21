
#define RGFW_EXPORT
//#define RGFW_ALLOC_DROPFILES
//#define RGFW_PRINT_ERRORS
#define RGFW_OPENGL

#include "RGFW.h"

#ifdef RGFW_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define OEMRESOURCE
    #include <windows.h>
#endif

#if !defined(__APPLE__) && !defined(RGFW_NO_GL_HEADER)
    #include <GL/gl.h>
#elif defined(__APPLE__)
    #ifndef GL_SILENCE_DEPRECATION
        #define GL_SILENCE_DEPRECATION
    #endif
    #include <OpenGL/gl.h>
    #include <OpenGL/OpenGL.h>
#elif defined(RGFW_EGL)
    #include <EGL/egl.h>
#endif
