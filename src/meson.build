frida_dep = subproject('frida-gum').get_variable('frida_dep')
sdk_dep = subproject('source-sdk').get_variable('sdk_dep')

add_project_arguments('-fpermissive', language: ['cpp'])
shared_library(
  'injectable',
  files(
    'main.cpp',
    'plugin.cpp',
    'gum/interceptor.cpp',
  ),
  include_directories: ['include'],
  dependencies: [frida_dep, sdk_dep],
  install : true
)
