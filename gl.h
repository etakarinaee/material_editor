#ifndef MATERIAL_EDITOR_GL_H
#define MATERIAL_EDITOR_GL_H

#include "linalg.h"

#define GL_OPAQUE 1.0f

void gl_clear(float r, float g, float b, float a);

void gl_set_viewport(int width, int height);

// sets the camera state for the current frame
void gl_begin(struct vector3 camera_position, struct vector3 look_at, float aspect_ratio);

#endif //MATERIAL_EDITOR_GL_H
