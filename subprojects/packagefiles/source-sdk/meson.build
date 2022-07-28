project('source-sdk-2013', 'cpp', default_options : [
  'warning_level=3',
])
cxx = meson.get_compiler('cpp')

sdk_dir = '.'
sdk_cflags = [
  '-D_LINUX',
  '-DLINUX',
  '-D_DLL_EXT=.so',
  '-DVPROF_LEVEL=1',
  '-D_finite=finite',
  '-Dstricmp=strcasecmp',
  '-D_stricmp=strcasecmp',
  '-D_strnicmp=strncasecmp',
  '-Dstrnicmp=strncasecmp',
  '-D_vsnprintf=vsnprintf',
  '-D_alloca=alloca',
  '-Dstrcmpi=strcasecmp',
  '-DPOSIX',
  '-DGNUC',
  '-DCOMPILER_GCC',
  '-DNO_MALLOC_OVERRIDE',

  '-fno-strict-aliasing',
  '-Wsign-compare',
  '-Wno-conversion',
  '-Wno-overloaded-virtual',
  '-Wno-non-virtual-dtor',
  '-Wno-invalid-offsetof',
  '-Wno-unknown-pragmas',
  '-Wno-unused',

  '-Wno-delete-non-virtual-dtor',
  '-Wno-narrowing',

]
sdk_libdirs = [ meson.current_source_dir() / 'libraries' ]
sdk_headers = include_directories(
  sdk_dir / 'mathlib',
  sdk_dir / 'public',
  sdk_dir / 'common',
  sdk_dir / 'public/tier0',
  sdk_dir / 'public/tier1',
  sdk_dir / 'tier1',
  sdk_dir,
  is_system : true
)

tier1_sources = [
	sdk_dir / 'tier1/bitbuf.cpp',
	sdk_dir / 'tier1/byteswap.cpp',
	sdk_dir / 'tier1/characterset.cpp',
	sdk_dir / 'tier1/checksum_crc.cpp',
	sdk_dir / 'tier1/checksum_md5.cpp',
	sdk_dir / 'tier1/commandbuffer.cpp',
	sdk_dir / 'tier1/convar.cpp',
	sdk_dir / 'tier1/datamanager.cpp',
	sdk_dir / 'tier1/diff.cpp',
	sdk_dir / 'tier1/generichash.cpp',
	sdk_dir / 'tier1/interface.cpp',
	sdk_dir / 'tier1/KeyValues.cpp',
	sdk_dir / 'tier1/mempool.cpp',
	sdk_dir / 'tier1/NetAdr.cpp',
	sdk_dir / 'tier1/newbitbuf.cpp',
	sdk_dir / 'tier1/processor_detect.cpp',
	sdk_dir / 'tier1/rangecheckedvar.cpp',
	sdk_dir / 'tier1/stringpool.cpp',
	sdk_dir / 'tier1/strtools.cpp',
	sdk_dir / 'tier1/tier1.cpp',
	sdk_dir / 'tier1/tokenreader.cpp',
	sdk_dir / 'tier1/undiff.cpp',
	sdk_dir / 'tier1/uniqueid.cpp',
	sdk_dir / 'tier1/utlbuffer.cpp',
	sdk_dir / 'tier1/utlbufferutil.cpp',
	sdk_dir / 'tier1/utlstring.cpp',
	sdk_dir / 'tier1/utlsymbol.cpp',
]
tier1_dep = static_library(
  'tier1',
  tier1_sources,
  c_args: sdk_cflags,
  cpp_args: sdk_cflags,
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
