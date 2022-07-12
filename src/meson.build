add_project_arguments('-fpermissive', language: ['cpp'])
shared_library(
  'injectable',
  files(
    'main.cpp',
    'interceptor.cpp',
    'plugin.cpp',
  ),
  include_directories: ['include'],
  dependencies: [frida_dep, sdk_dep],
  install : true
)
