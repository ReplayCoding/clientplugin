frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')
tier1_dep = subproject('source-sdk').get_variable('tier1_dep')

install_data('ClientPlugin.vdf', install_dir: 'addons')
shared_library(
  'clientplugin',
  files(
    'interfaces.cpp',
    'plugin.cpp',
    'modules.cpp',
    'modules/killfeedmod.cpp',
    'modules/videorecordmod.cpp',
    'gum/interceptor.cpp',
  ),
  include_directories: ['include'],
  dependencies: [frida_dep, sdk_dep],
  link_with: [tier1_dep],
  install_dir: 'addons',
  install : true
)
