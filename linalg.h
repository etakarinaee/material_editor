#ifndef MATERIAL_EDITOR_MATH_H
#define MATERIAL_EDITOR_MATH_H

#include <math.h>

#define PI 3.14159265358979f

struct vector3 {
    float x, y, z;
};

struct matrix4 {
    float m[16];
};

static inline struct vector3 vector3(const float x, const float y, const float z) {
    return (struct vector3){x, y, z};
}

static inline struct vector3 vector3_add(const struct vector3 a, const struct vector3 b) {
    return vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline struct vector3 vector3_sub(const struct vector3 a, const struct vector3 b) {
    return vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline struct vector3 vector3_cross(const struct vector3 a, const struct vector3 b) {
    return vector3(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}

static inline float vector3_dot(const struct vector3 a, const struct vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline struct vector3 vector3_normalize(const struct vector3 v) {
    const float len = sqrtf(vector3_dot(v, v));
    return vector3(v.x / len, v.y / len, v.z / len);
}

static inline struct matrix4 matrix4_look_at(const struct vector3 eye, const struct vector3 target) {
    const struct vector3 up = vector3(0, 1, 0);
    const struct vector3 f = vector3_normalize(vector3_sub(target, eye));
    const struct vector3 s = vector3_normalize(vector3_cross(f, up));
    const struct vector3 u = vector3_cross(s, f);

    struct matrix4 r;
    r.m[0] = s.x;
    r.m[1] = u.x;
    r.m[2] = -f.x;
    r.m[3] = 0.0f;
    r.m[4] = s.y;
    r.m[5] = u.y;
    r.m[6] = -f.y;
    r.m[7] = 0.0f;
    r.m[8] = s.z;
    r.m[9] = u.z;
    r.m[10] = -f.z;
    r.m[11] = 0.0f;
    r.m[12] = -vector3_dot(s, eye);
    r.m[13] = -vector3_dot(u, eye);
    r.m[14] = vector3_dot(f, eye);
    r.m[15] = 1.0f;
    return r;
}

static inline struct matrix4 matrix4_perspective(const float fov_degrees, const float aspect, const float near,
                                                 const float far) {
    const float t = 1.0f / tanf(fov_degrees * PI / 360.0f);

    struct matrix4 r;
    r.m[0] = t / aspect;
    r.m[1] = 0.0f;
    r.m[2] = 0.0f;
    r.m[3] = 0.0f;
    r.m[4] = 0.0f;
    r.m[5] = t;
    r.m[6] = 0.0f;
    r.m[7] = 0.0f;
    r.m[8] = 0.0f;
    r.m[9] = 0.0f;
    r.m[10] = (far + near) / (near - far);
    r.m[11] = -1.0f;
    r.m[12] = 0.0f;
    r.m[13] = 0.0f;
    r.m[14] = (2.0f * far * near) / (near - far);
    r.m[15] = 0.0f;
    return r;
}

#endif //MATERIAL_EDITOR_MATH_H
