#include "gl.h"

#include <stdio.h>
#include <GL/glew.h>

#define SPHERE_STACKS 64
#define SPHERE_SLICES 128

static const char *vertex_shader_source =
        "#version 330 core\n"
        "layout(location = 0) in vec3 a_position;\n"
        "uniform mat4 u_model;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "void main() {\n"
        "    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);\n"
        "}\n";

static const char *fragment_shader_source =
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

// a sphere mesh
static void mesh1_create(void) {
    // a sphere is sliced like an orange, so slices is longitude, and then rings are stacked from top to bottom, so it
    // is latitude
    float vertices[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1) * 3];
    unsigned int indices[SPHERE_STACKS * SPHERE_SLICES * 6];

    int v = 0;
    for (int stack = 0; stack <= SPHERE_STACKS; stack++) {
        // angle from top, where 0 is at the North Pole and PI at South Pole
        const float phi = PI * stack / SPHERE_STACKS;
        // height on y-axis
        const float y = cosf(phi);
        // radius at this height
        const float r = sinf(phi);

        for (int slice = 0; slice <= SPHERE_SLICES; slice++) {
            // angle around y-axis from 0 to 2*PI
            const float theta = 2.0f * PI * slice / SPHERE_SLICES;
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

    const unsigned int vertex = shader_create(GL_VERTEX_SHADER, vertex_shader_source);
    const unsigned int fragment = shader_create(GL_FRAGMENT_SHADER, fragment_shader_source);

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
}

void gl_clear(const float r, const float g, const float b, const float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_set_viewport(const int x, const int y, const int width, const int height) {
    glViewport(x, y, width, height);
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
