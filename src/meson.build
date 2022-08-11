frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')
x264_dep = subproject('x264', default_options: ['warning_level=0']).get_variable('libx264_dep')
imgui_dep = subproject('imgui').get_variable('imgui_dep')
gl_dep = dependency('gl')

cflags = [ '-Wno-pedantic' ]

install_data('ClientPlugin.vdf', install_dir: 'addons', install_tag: 'addon')
shared_library(
  'clientplugin',
  files(
    'interfaces.cpp',
    'plugin.cpp',
    'modules/modules.cpp',
    'modules/killfeedmod.cpp',
    'modules/videorecord/videorecordmod.cpp',
    'modules/videorecord/x264encoder.cpp',
    'modules/gfxoverlaymod.cpp',
    'hook/gum/interceptor.cpp',
    'hook/gum/x86patcher.cpp',
  ),
  c_args: cflags,
  cpp_args: cflags,
  include_directories: inc_dirs,
  dependencies: [frida_dep, sdk_dep, x264_dep, imgui_dep, gl_dep],
  install_dir: 'addons',
  install_tag: 'addon',
  install : true
)
