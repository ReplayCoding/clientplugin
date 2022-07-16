frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')

add_project_arguments('-fpermissive', language: ['cpp'])
install_data('ClientPlugin.vdf', install_dir: 'addons')
shared_library(
  'clientplugin',
  files(
    'interfaces.cpp',
    'plugin.cpp',
    'modules.cpp',
    'modules/killfeedmod.cpp',
    'gum/interceptor.cpp',
  ),
  include_directories: ['include'],
  dependencies: [frida_dep, sdk_dep],
  install_dir: 'addons',
  install : true
)
