#ifndef MATERIAL_EDITOR_GL_H
#define MATERIAL_EDITOR_GL_H

#include "linalg.h"
#include "result.h"

#define GL_OPAQUE 1.0f

struct gl_glyph {
    // texture x offset in atlas
    // [0..1]
    float tx;
    // texture y offset in atlas
    // [0..1]
    float ty;
    // texture width in atlas
    // [0..1]
    float tw;
    // texture height in atlas
    // [0..1]
    float th;
    // bitmap width in pixels
    int width;
    // bitmap height in pixels
    int height;
    // left bearing
    int bearing_x;
    // top bearing
    int bearing_y;
    // horizontal advance in pixels
    int advance;
};

struct gl_font {
    unsigned int atlas_texture;
    int atlas_width;
    int atlas_height;
    struct gl_glyph glyphs[128];
    // used for the cursor
    float line_height;

    unsigned int program;
    unsigned int vao;
    unsigned int vbo;
    int u_projection;
    int u_text_color;
};

material_editor_result gl_font_initialize(struct gl_font *font, const char *ttf, int pixel_height);

void gl_font_draw(const struct gl_font *font, const char *string, float x, float y, float scale, struct vector3 color,
                  int screen_width, int screen_height);

void gl_font_draw_cursor(const struct gl_font *font, const char *string, int cursor,
                         float x, float y, float scale,
                         struct vector3 color,
                         int screen_width, int screen_height);

void gl_draw_rect(float x, float y, float width, float height,
                  struct vector3 color,
                  int screen_width, int screen_height);

void gl_initialize(void);

void gl_clear(float r, float g, float b, float a);

void gl_set_viewport(int x, int y, int width, int height);

void gl_set_scissor(int x, int y, int width, int height);

void gl_clear_scissor(void);

// sets the camera state for the current frame
void gl_begin(struct vector3 camera_position, struct vector3 look_at, float aspect_ratio);

// sphere
void gl_draw_mesh1(struct vector3 position, float radius, struct vector3 color);

#endif //MATERIAL_EDITOR_GL_H
