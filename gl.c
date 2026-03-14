#include "gl.h"

#include <GL/glew.h>

static struct {
    struct matrix4 view;
    struct matrix4 projection;
    struct vector3 camera_position;
} gl;

void gl_clear(const float r, const float g, const float b, const float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_set_viewport(const int width, const int height) {
    glViewport(0, 0, width, height);
}

void gl_begin(const struct vector3 camera_position, const struct vector3 look_at, const float aspect_ratio) {
    gl.camera_position = camera_position;
    gl.view = matrix4_look_at(camera_position, look_at);
    gl.projection = matrix4_perspective(45.0f, aspect_ratio, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);
}
