frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')
x264_dep = subproject('x264', default_options: [
  'asm=enabled',
  'cli=false',
  'warning_level=0'
]).get_variable('libx264_dep')
imgui_dep = subproject('imgui').get_variable('imgui_dep')
# fmt fails to link and i have no idea why but who cares LOL. deal with it.
fmt_dep = subproject('fmt').get_variable('fmt_header_only_dep')
gl_dep = dependency('gl')

cflags = [
  '-Wno-vla', # really?
]

install_data('ClientPlugin.vdf', install_dir: 'addons', install_tag: 'addon')
plugin_lib = shared_library(
  'clientplugin',
  files(
    'interfaces.cpp',
    'plugin.cpp',
    'offsets.cpp',
    'x264encoder.cpp',
    'modules/modules.cpp',
    'modules/unhidecvars.cpp',
    'modules/killfeed.cpp',
    'modules/gfxoverlay.cpp',
    'modules/videorecord.cpp',
    'hook/gum/interceptor.cpp',
    'hook/gum/x86patcher.cpp',
  ),
  c_args: cflags,
  cpp_args: cflags,
  include_directories: include_directories('.'),
  dependencies: [frida_dep, sdk_dep, x264_dep, imgui_dep, gl_dep, fmt_dep],
  # We want to remove the rpath first
  install: false,
)

custom_target(
  'clientplugin-fixup',
  output: plugin_lib.name() + '.so',
  input: plugin_lib,
  command: ['patchelf', '--remove-rpath', '@INPUT@', '--output', '@OUTPUT@'],
  install_dir: 'addons',
  install_tag: 'addon',
  install : true,
)
