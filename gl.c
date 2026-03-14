#include "gl.h"

#include <GL/glew.h>

void gl_clear(const float r, const float g, const float b, const float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_set_viewport(const int width, const int height) {
    glViewport(0, 0, width, height);
}