project('material_editor', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c11'])

x11_dep = dependency('x11')
gl_dep = dependency('gl')
glew_dep = dependency('glew')
m_dep = meson.get_compiler('c').find_library('m', required : false)

material_editor = executable('material_editor',
                             'main.c',
                             'platform_x11.c',
                             'gl.c',
                             dependencies : [x11_dep, gl_dep, glew_dep, m_dep],
                             install : true)

test('test', material_editor)