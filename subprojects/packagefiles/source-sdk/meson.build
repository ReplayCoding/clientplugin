project('source-sdk-2013', 'cpp', default_options : [
  'warning_level=3',
  'werror=true',
])
cxx = meson.get_compiler('cpp')

sdk_dir = '.'
sdk_cflags = [
  '-DGNUC',
  '-DCOMPILER_GCC',
  '-D_LINUX', '-DLINUX',
  '-DPOSIX', '-D_POSIX',
  '-DNO_HOOK_MALLOC', '-DNO_MALLOC_OVERRIDE',
  '-DUSE_SDL', '-DDX_TO_GL_ABSTRACTION',
  '-D_GLIBCXX_USE_CXX11_ABI=0',

  # This is annoying but otherwise there are warnings
  '-Wno-non-virtual-dtor',
  '-Wno-unused-parameter',
]
sdk_libdirs = [
  meson.current_source_dir() / 'libraries',
  meson.current_source_dir() / 'lib/public/linux32'
]
sdk_headers = include_directories(
  sdk_dir / 'public',
  sdk_dir / 'common',
  sdk_dir / 'public/tier0',
  sdk_dir / 'public/tier1',
  sdk_dir,
)

tier1_sources = [
  sdk_dir /	'tier1/bitbuf.cpp',
  sdk_dir /	'tier1/newbitbuf.cpp',
  sdk_dir /	'tier1/byteswap.cpp',
  sdk_dir /	'tier1/characterset.cpp',
  sdk_dir /	'tier1/checksum_crc.cpp',
  sdk_dir /	'tier1/checksum_md5.cpp',
  sdk_dir /	'tier1/checksum_sha1.cpp',
  sdk_dir /	'tier1/commandbuffer.cpp',
  sdk_dir /	'tier1/convar.cpp',
  sdk_dir /	'tier1/datamanager.cpp',
  sdk_dir /	'tier1/diff.cpp',
  sdk_dir /	'tier1/generichash.cpp',
  sdk_dir /	'tier1/ilocalize.cpp',
  sdk_dir /	'tier1/interface.cpp',
  sdk_dir /	'tier1/KeyValues.cpp',
  sdk_dir /	'tier1/kvpacker.cpp',
  sdk_dir /	'tier1/lzmaDecoder.cpp',
  # sdk_dir /	"lzss.cpp" [!$SOURCESDK]
  sdk_dir /	'tier1/mempool.cpp',
  sdk_dir /	'tier1/memstack.cpp',
  sdk_dir /	'tier1/NetAdr.cpp',
  sdk_dir /	'tier1/splitstring.cpp',
  # sdk_dir /	"processor_detect.cpp" [$WINDOWS||$X360]
  
  # [$POSIX]
  sdk_dir /	'tier1/processor_detect_linux.cpp',
  
  # [$LINUXALL||$PS3]
  sdk_dir /	'tier1/qsort_s.cpp',

  sdk_dir /	'tier1/rangecheckedvar.cpp',
  sdk_dir /	'tier1/reliabletimer.cpp',
  sdk_dir /	'tier1/stringpool.cpp',
  sdk_dir /	'tier1/strtools.cpp',
  sdk_dir /	'tier1/strtools_unicode.cpp',
  sdk_dir /	'tier1/tier1.cpp',
  sdk_dir /	'tier1/tokenreader.cpp',
  sdk_dir /	'tier1/sparsematrix.cpp',
  sdk_dir /	'tier1/uniqueid.cpp',
  sdk_dir /	'tier1/utlbuffer.cpp',
  sdk_dir /	'tier1/utlbufferutil.cpp',
  sdk_dir /	'tier1/utlstring.cpp',
  sdk_dir /	'tier1/utlsymbol.cpp',
  sdk_dir /	'tier1/utlbinaryblock.cpp',

  # [$LINUXALL]
  sdk_dir /	'tier1/pathmatch.cpp',

  sdk_dir /	'tier1/snappy.cpp',
  sdk_dir /	'tier1/snappy-sinksource.cpp',
  sdk_dir /	'tier1/snappy-stubs-internal.cpp',
]

tier1_cflags = [
  '-Wno-pedantic',
  '-Wno-implicit-fallthrough',
  '-Wno-register',
  '-Wno-sign-compare',
  '-Wno-reorder',
  '-Wno-misleading-indentation',
  '-Wno-nonnull-compare',
  '-Wno-strict-aliasing',
  '-Wno-unused-function',
  '-Wno-unused-result',
] + sdk_cflags

tier1_dep = static_library(
  'tier1',
  tier1_sources,
  c_args: tier1_cflags,
  cpp_args: tier1_cflags,
  include_directories: sdk_headers,
)

sdk_dep = declare_dependency(
  compile_args: sdk_cflags,
  dependencies: [
    # These aren't included in the SDK as source code
    # tier0 is a shared lib so we need to fixup the soname
    # we will also need to do this for vstdlib if
    # we ever use it.
    cxx.find_library('tier0', dirs: sdk_libdirs),
    # cxx.find_library('vstdlib', dirs: sdk_libdirs),
    cxx.find_library('tier2', dirs: sdk_libdirs),
    cxx.find_library('tier3', dirs: sdk_libdirs),
    cxx.find_library('bitmap', dirs: sdk_libdirs),
  ],
  link_with: [
    # tier1 is borked so we compile it
    tier1_dep,
  ],
  include_directories: sdk_headers,
)
