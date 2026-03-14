project('material_editor', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c11'])

x11_dep = dependency('x11')

material_editor = executable('material_editor',
                             'main.c',
                             'platform_x11.c',
                             dependencies : [x11_dep],
                             install : true)

test('test', material_editor)