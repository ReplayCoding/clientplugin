project('sdk-plugin', 'c', 'cpp',
  version : 'dev',
  default_options :
    [
      'warning_level=3',
      'werror=true',
      'cpp_std=gnu++20',
      'c_std=gnu99',

      'default_library=static',
      'b_ndebug=if-release',
      'buildtype=debugoptimized'
    ]
)

if get_option('buildtype') == 'debug'
  error('x264 is fucked, use debugoptimized or release')
endif
subdir('src')
