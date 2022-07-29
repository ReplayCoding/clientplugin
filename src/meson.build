frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')
x264_dep = subproject('x264', default_options: ['warning_level=0']).get_variable('libx264_dep')

cflags = [ '-Wno-pedantic' ]

install_data('ClientPlugin.vdf', install_dir: 'addons')
shared_library(
  'clientplugin',
  files(
    'interfaces.cpp',
    'plugin.cpp',
    'modules/modules.cpp',
    'modules/killfeedmod.cpp',
    'modules/videorecordmod.cpp',
    'gum/interceptor.cpp',
    'gum/x86patcher.cpp',
  ),
  include_directories: inc_dirs,
  c_args: cflags,
  cpp_args: cflags,
  dependencies: [frida_dep, sdk_dep, x264_dep],
  install_dir: 'addons',
  install : true
)
