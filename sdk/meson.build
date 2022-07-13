cxx = meson.get_compiler('cpp')

sdk_dir = 'source-sdk-2013/mp/src'
sdk_cflags = [
  # Yoinked from cathook
  '-D_POSIX=1',
  '-DFREETYPE_GL_USE_VAO=1',
  '-DRAD_TELEMETRY_DISABLED=1',
  '-DLINUX=1',
  '-DUSE_SDL=1',
  '-D_LINUX=1',
  '-DPOSIX=1',
  '-DGNUC=1',
  '-DNO_MALLOC_OVERRIDE=1',
  '-D_GLIBCXX_USE_CXX11_ABI=0',
]
sdk_libdirs = [
  meson.current_source_dir() / sdk_dir / 'lib/public/linux32',
]
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
  # sdk_dir /	'tier1/lzmaDecoder.cpp',
  # $File	"lzss.cpp" [!$SOURCESDK]
  sdk_dir /	'tier1/mempool.cpp',
  sdk_dir /	'tier1/memstack.cpp',
  sdk_dir /	'tier1/NetAdr.cpp',
  sdk_dir /	'tier1/splitstring.cpp',
  # $File	"processor_detect.cpp" [$WINDOWS||$X360]
  
  # $File	"processor_detect_linux.cpp"	[$POSIX]
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
  # $File	"pathmatch.cpp" [$LINUXALL]
  sdk_dir /	'tier1/snappy.cpp',
  sdk_dir /	'tier1/snappy-sinksource.cpp',
  sdk_dir /	'tier1/snappy-stubs-internal.cpp',
]

sdk_dep = declare_dependency(
  # Workaround for a dumb meson thing
  sources: tier1_sources,

  compile_args: sdk_cflags,
  dependencies: [
    # These aren't included in the SDK as source code
    cxx.find_library('tier0', dirs: sdk_libdirs),
    cxx.find_library('tier2', dirs: sdk_libdirs),
    cxx.find_library('vstdlib', dirs: sdk_libdirs),
    # tier1 is borked so we compile it
  ],
  include_directories: sdk_headers,
)
