project('frida-gum', 'c')

cc = meson.get_compiler('c')
frida_dep = declare_dependency(
  include_directories: include_directories('.', is_system : true),
  dependencies: cc.find_library('frida-gum', dirs: [meson.current_source_dir()]),
)
