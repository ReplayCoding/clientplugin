frida_dir = 'frida-gum/'
frida_dep = declare_dependency(
  compile_args: ['-D_GLIBCXX_USE_CXX11_ABI=0'],
  include_directories: include_directories(frida_dir, is_system : true),
  link_args: ['-L' + meson.current_source_dir() / frida_dir, '-lfrida-gum'],
)

sdk_dir = 'source-sdk-2013/'
sdk_dep = declare_dependency(
    include_directories: include_directories(
      sdk_dir / 'mp/src/mathlib',
      sdk_dir / 'mp/src/public',
      sdk_dir / 'mp/src/common',
      sdk_dir / 'mp/src/public/tier0',
      sdk_dir / 'mp/src/public/tier1',
      sdk_dir / 'mp/src/tier1',
      sdk_dir / 'mp/src',
      is_system : true
    ),
  )
