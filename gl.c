#include "gl.h"

#include <stdio.h>
#include <math.h>
#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define SPHERE_STACKS 64
#define SPHERE_SLICES 128

static const char *mesh_vertex_shader_source =
        "#version 330 core\n"
        "layout(location = 0) in vec3 a_position;\n"
        "uniform mat4 u_model;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "void main() {\n"
        "    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);\n"
        "}\n";

static const char *mesh_fragment_shader_source =
        "#version 330 core\n"
        "uniform vec3 u_color;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    frag_color = vec4(u_color, 1.0);\n"
        "}\n";

static struct {
    struct matrix4 view;
    struct matrix4 projection;
    struct vector3 camera_position;

    unsigned int program;
    int u_model;
    int u_view;
    int u_projection;
    int u_color;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    int indices;
} gl;

static unsigned int shader_create(const unsigned int type, const char *source) {
    const unsigned int shader = glCreateShader(type);
    const int count = 1;
    const GLint *length = 0;
    glShaderSource(shader, count, &source, length);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        const int size = 512;
        char log[size];
        GLsizei *length = 0;

        glGetShaderInfoLog(shader, size, length, log);
        fprintf(stderr, "%s\n", log);
    }

    return shader;
}

static const char *text_vertex_source =
        "#version 330 core\n"
        "layout(location = 0) in vec4 vertex;\n"
        "out vec2 tex_coords;\n"
        "uniform mat4 u_projection;\n"
        "void main() {\n"
        "    gl_Position = u_projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "    tex_coords = vertex.zw;\n"
        "}\n";

static const char *text_fragment_source =
        "#version 330 core\n"
        "in vec2 tex_coords;\n"
        "out vec4 frag_color;\n"
        "uniform sampler2D u_text;\n"
        "uniform vec3 u_text_color;\n"
        "void main() {\n"
        "    float dist = texture(u_text, tex_coords).r;\n"
        "    float edge = 0.5;\n"
        "    float aa_width = fwidth(dist) * 0.75;\n"
        "    float a = smoothstep(edge - aa_width, edge + aa_width, dist);\n"
        "    if (a < 0.01) discard;\n"
        "    frag_color = vec4(u_text_color, a);\n"
        "}\n";

#define GLYPH_PADDING 2

material_editor_result gl_font_initialize(struct gl_font *font, const char *ttf, const int pixel_height) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        return RESULT_TTF_ERROR_INIT;
    }

    FT_Face face;

    const int face_index = 0;
    if (FT_New_Face(ft, ttf, face_index, &face)) {
        FT_Done_FreeType(ft);
        return RESULT_TTF_ERROR_LOAD;
    }

    const int pixel_width = 0;
    FT_Set_Pixel_Sizes(face, pixel_width, pixel_height);

    int atlas_width = 0;
    int atlas_height = 0;

    for (int i = 0; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_DEFAULT)) {
            continue;
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF)) {
            continue;
        }

        const int glyph_width = (int) face->glyph->bitmap.width;
        const int glyph_height = (int) face->glyph->bitmap.rows;

        atlas_width += glyph_width + GLYPH_PADDING;

        if (glyph_height > atlas_height) {
            atlas_height = glyph_height;
        }
    }

    font->atlas_width = atlas_width;
    font->atlas_height = atlas_height;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const int n = 1;
    glGenTextures(n, &font->atlas_texture);
    glBindTexture(GL_TEXTURE_2D, font->atlas_texture);

    const int level = 0;
    const int border = 0;
    unsigned char *zeros = calloc(atlas_width * atlas_height, 1);
    glTexImage2D(GL_TEXTURE_2D, level, GL_RED, atlas_width, atlas_height, border,
                 GL_RED, GL_UNSIGNED_BYTE, zeros);
    free(zeros);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int xoffset = 0;
    memset(font->glyphs, 0, sizeof(font->glyphs));

    for (int i = 0; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_DEFAULT)) {
            continue;
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF)) {
            continue;
        }

        const FT_Bitmap *bitmap = &face->glyph->bitmap;
        const int glyph_width = (int) bitmap->width;
        const int glyph_height = (int) bitmap->rows;

        if (glyph_width > 0 && glyph_height > 0) {
            const int yoffset = 0;
            glPixelStorei(GL_UNPACK_ROW_LENGTH, bitmap->pitch);
            glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset,
                            (GLsizei) glyph_width, (GLsizei) glyph_height,
                            GL_RED, GL_UNSIGNED_BYTE, bitmap->buffer);
        }

        struct gl_glyph *glyph = &font->glyphs[i];

        glyph->width = glyph_width;
        glyph->height = glyph_height;
        glyph->bearing_x = face->glyph->bitmap_left;
        glyph->bearing_y = face->glyph->bitmap_top;
        glyph->advance = (int) (face->glyph->advance.x >> 6);
        glyph->tx = (float) xoffset / (float) atlas_width;
        glyph->ty = 0.0f;
        glyph->tw = (float) glyph_width / (float) atlas_width;
        glyph->th = (float) glyph_height / (float) atlas_height;

        xoffset += glyph_width + GLYPH_PADDING;
    }

    font->line_height = (float) font->glyphs['A'].height * 1.3f;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    const unsigned int vertex = shader_create(GL_VERTEX_SHADER, text_vertex_source);
    const unsigned int fragment = shader_create(GL_FRAGMENT_SHADER, text_fragment_source);

    font->program = glCreateProgram();
    glAttachShader(font->program, vertex);
    glAttachShader(font->program, fragment);
    glLinkProgram(font->program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    font->u_projection = glGetUniformLocation(font->program, "u_projection");
    font->u_text_color = glGetUniformLocation(font->program, "u_text_color");

    glGenVertexArrays(n, &font->vao);
    glGenBuffers(n, &font->vbo);

    glBindVertexArray(font->vao);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);

    const void *data = 0;
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, data, GL_DYNAMIC_DRAW);
    const int index = 0;
    glEnableVertexAttribArray(index);
    const int size = 4;
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    const int buffer = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    const int array = 0;
    glBindVertexArray(array);

    return RESULT_SUCCESS;
}

void gl_font_draw(const struct gl_font *font, const char *string, float x, float y, const float scale,
                  const struct vector3 color,
                  const int screen_width, const int screen_height) {
    float projection[16] = {0};
    const float right = (float) screen_width;
    const float top = (float) screen_height;
    projection[0] = 2.0f / right;
    projection[5] = 2.0f / top;
    projection[10] = -1.0f;
    projection[12] = -1.0f;
    projection[13] = -1.0f;
    projection[15] = 1.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(font->program);
    glUniformMatrix4fv(font->u_projection, 1, GL_FALSE, projection);
    glUniform3f(font->u_text_color, color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->atlas_texture);
    glBindVertexArray(font->vao);

    const float start_x = x;
    const float line_height = (float) font->glyphs['A'].height * scale * 1.3f;

    for (const char *c = string; *c; c++) {
        const int ch = (unsigned char) *c;

        if (ch == '\n') {
            x = start_x;
            y -= line_height;
            continue;
        }

        if (ch >= 128) continue;

        const struct gl_glyph *glyph = &font->glyphs[ch];

        const float xpos = x + (float) glyph->bearing_x * scale;
        const float ypos = y - (float) (glyph->height - glyph->bearing_y) * scale;
        const float width = (float) glyph->width * scale;
        const float height = (float) glyph->height * scale;

        const float vertices[6][4] = {
            {xpos, ypos + height, glyph->tx, glyph->ty},
            {xpos, ypos, glyph->tx, glyph->ty + glyph->th},
            {xpos + width, ypos, glyph->tx + glyph->tw, glyph->ty + glyph->th},

            {xpos, ypos + height, glyph->tx, glyph->ty},
            {xpos + width, ypos, glyph->tx + glyph->tw, glyph->ty + glyph->th},
            {xpos + width, ypos + height, glyph->tx + glyph->tw, glyph->ty},
        };

        glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
        const int offset = 0;
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertices), vertices);
        const int buffer = 0;
        glBindBuffer(GL_ARRAY_BUFFER, buffer);

        const int first = 0;
        const int count = 6;
        glDrawArrays(GL_TRIANGLES, first, count);

        x += (float) glyph->advance * scale;
    }

    const int array = 0;
    glBindVertexArray(array);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void gl_font_draw_cursor(const struct gl_font *font, const char *string, int cursor, float x, float y,
                         const float scale,
                         const struct vector3 color, const int screen_width, const int screen_height) {
    const float start_x = x;
    const float line_height = font->line_height * scale;

    for (int i = 0; i < cursor && string[i]; i++) {
        const int ch = (unsigned char) string[i];

        if (ch == '\n') {
            x = start_x;
            y -= line_height;
            continue;
        }

        if (ch < 128) {
            x += (float) font->glyphs[ch].advance * scale;
        }
    }

    const float cursor_width = 2.0f;
    const float cursor_height = line_height;
    const float ascender = (float) font->glyphs['A'].bearing_y * scale;
    const float cursor_y = y - (line_height - ascender);

    gl_draw_rect(x, cursor_y, cursor_width, cursor_height, color, screen_width, screen_height);
}

static const char *rect_vertex_source =
        "#version 330 core\n"
        "layout(location = 0) in vec2 a_position;\n"
        "uniform mat4 u_projection;\n"
        "void main() {\n"
        "    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);\n"
        "}\n";

static const char *rect_fragment_source =
        "#version 330 core\n"
        "uniform vec3 u_color;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    frag_color = vec4(u_color, 1.0);\n"
        "}\n";


static struct {
    unsigned int program;
    int u_projection;
    int u_color;
    unsigned int vao;
    unsigned int vbo;
} gl_rect;

void rect_initialize(void) {
    const unsigned int vertex = shader_create(GL_VERTEX_SHADER, rect_vertex_source);
    const unsigned int fragment = shader_create(GL_FRAGMENT_SHADER, rect_fragment_source);

    gl_rect.program = glCreateProgram();
    glAttachShader(gl_rect.program, vertex);
    glAttachShader(gl_rect.program, fragment);
    glLinkProgram(gl_rect.program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    gl_rect.u_projection = glGetUniformLocation(gl_rect.program, "u_projection");
    gl_rect.u_color = glGetUniformLocation(gl_rect.program, "u_color");

    glGenVertexArrays(1, &gl_rect.vao);
    glGenBuffers(1, &gl_rect.vbo);

    glBindVertexArray(gl_rect.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl_rect.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_DYNAMIC_DRAW);
    const int index = 0;
    glEnableVertexAttribArray(index);
    const int size = 2;
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    const int buffer = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    const int array = 0;
    glBindVertexArray(array);
}

void gl_draw_rect(const float x, const float y, const float width, const float height,
                  const struct vector3 color,
                  const int screen_width, const int screen_height) {
    float projection[16] = {0};
    projection[0] = 2.0f / (float) screen_width;
    projection[5] = 2.0f / (float) screen_height;
    projection[10] = -1.0f;
    projection[12] = -1.0f;
    projection[13] = -1.0f;
    projection[15] = 1.0f;

    const float vertices[12] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y,
        x + width, y + height,
        x, y + height,
    };

    glDisable(GL_DEPTH_TEST);

    glUseProgram(gl_rect.program);
    const int matrix_count = 1;
    glUniformMatrix4fv(gl_rect.u_projection, matrix_count, GL_FALSE, projection);
    glUniform3f(gl_rect.u_color, color.x, color.y, color.z);

    glBindVertexArray(gl_rect.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl_rect.vbo);
    const int offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertices), vertices);

    const int first = 0;
    int arrays_count = 6;
    glDrawArrays(GL_TRIANGLES, first, arrays_count);

    const int buffer = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    const int array = 0;
    glBindVertexArray(array);
    glEnable(GL_DEPTH_TEST);
}

static void mesh1_create(void) {
    // a sphere is sliced like an orange, so slices is longitude, and then rings are stacked from top to bottom, so it
    // is latitude
    float vertices[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1) * 3];
    unsigned int indices[SPHERE_STACKS * SPHERE_SLICES * 6];

    int v = 0;
    for (int stack = 0; stack <= SPHERE_STACKS; stack++) {
        // angle from top, where 0 is at the North Pole and PI at South Pole
        const float phi = PI * (float) stack / SPHERE_STACKS;
        // height on y-axis
        const float y = cosf(phi);
        // radius at this height
        const float r = sinf(phi);

        for (int slice = 0; slice <= SPHERE_SLICES; slice++) {
            // angle around y-axis from 0 to 2*PI
            const float theta = 2.0f * PI * (float) slice / SPHERE_SLICES;
            // position on XZ plane
            const float x = r * cosf(theta);
            const float z = r * sinf(theta);

            vertices[v++] = x;
            vertices[v++] = y;
            vertices[v++] = z;
        }
    }

    int i = 0;
    for (int stack = 0; stack < SPHERE_STACKS; stack++) {
        const int row = stack * (SPHERE_SLICES + 1);
        const int next = row + (SPHERE_SLICES + 1);

        for (int slice = 0; slice < SPHERE_SLICES; slice++) {
            // degenerate triangle at the top pole
            if (stack != 0) {
                indices[i++] = row + slice;
                indices[i++] = next + slice;
                indices[i++] = row + slice + 1;
            }

            // degenerate triangle at the bottom pole
            if (stack != SPHERE_STACKS - 1) {
                indices[i++] = row + slice + 1;
                indices[i++] = next + slice;
                indices[i++] = next + slice + 1;
            }
        }
    }

    gl.indices = i;

    const int n = 1;
    glGenVertexArrays(n, &gl.vao);
    glGenBuffers(n, &gl.vbo);
    glGenBuffers(n, &gl.ebo);

    glBindVertexArray(gl.vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    const int index = 0;
    const int size = 3;
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(index);

    const int array = 0;
    glBindVertexArray(array);
}

void gl_initialize(void) {
    glEnable(GL_MULTISAMPLE);

    const unsigned int vertex = shader_create(GL_VERTEX_SHADER, mesh_vertex_shader_source);
    const unsigned int fragment = shader_create(GL_FRAGMENT_SHADER, mesh_fragment_shader_source);

    gl.program = glCreateProgram();
    glAttachShader(gl.program, vertex);
    glAttachShader(gl.program, fragment);
    glLinkProgram(gl.program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    gl.u_model = glGetUniformLocation(gl.program, "u_model");
    gl.u_view = glGetUniformLocation(gl.program, "u_view");
    gl.u_projection = glGetUniformLocation(gl.program, "u_projection");
    gl.u_color = glGetUniformLocation(gl.program, "u_color");

    mesh1_create();
    rect_initialize();
}

void gl_clear(const float r, const float g, const float b, const float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_set_viewport(const int x, const int y, const int width, const int height) {
    glViewport(x, y, width, height);
}

void gl_set_scissor(const int x, const int y, const int width, const int height) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

void gl_clear_scissor(void) {
    glDisable(GL_SCISSOR_TEST);
}

void gl_begin(const struct vector3 camera_position, const struct vector3 look_at, const float aspect_ratio) {
    gl.camera_position = camera_position;
    gl.view = matrix4_look_at(camera_position, look_at);
    gl.projection = matrix4_perspective(45.0f, aspect_ratio, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);
}

void gl_draw_mesh1(const struct vector3 position, const float radius, const struct vector3 color) {
    const struct matrix4 model = matrix4_multiply(
        matrix4_multiply(
            matrix4_translate(position),
            matrix4_scale(radius)
        ),
        matrix4_scale_xyz(1.0f, 0.98f, 1.0f)
    );

    const int count = 1;
    const int transpose = GL_FALSE;

    glUseProgram(gl.program);
    glUniformMatrix4fv(gl.u_model, count, transpose, model.m);
    glUniformMatrix4fv(gl.u_view, count, transpose, gl.view.m);
    glUniformMatrix4fv(gl.u_projection, count, transpose, gl.projection.m);
    glUniform3f(gl.u_color, color.x, color.y, color.z);

    const void *indices = 0;
    const int array = 0;

    glBindVertexArray(gl.vao);
    glDrawElements(GL_TRIANGLES, gl.indices, GL_UNSIGNED_INT, indices);
    glBindVertexArray(array);
}
