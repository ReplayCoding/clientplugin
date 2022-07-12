project('frida-c', 'cpp',
  version : '0.1',
  default_options :
    [
      'warning_level=1',
      'cpp_std=c++20',

      'c_std=gnu99',
      'default_library=static',
      'optimization=s',
      'b_ndebug=true',
      'strip=true',
    ]
)

subdir('external')
subdir('src')
