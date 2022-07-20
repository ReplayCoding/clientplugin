frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')
x264_dep = subproject('x264').get_variable('libx264_dep')

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
  ),
  include_directories: inc_dirs,
  dependencies: [frida_dep, sdk_dep, x264_dep],
  install_dir: 'addons',
  install : true
)
