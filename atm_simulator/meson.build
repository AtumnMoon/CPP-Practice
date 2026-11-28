project(
  'atm_simulator',
  'cpp',
  version: '0.1.0',
  meson_version: '>=1.1.0',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',       # -Wall -Wextra -Wpedantic  |  /W4
    'werror=false',          # CI: meson setup build -Dwerror=true
    'b_ndebug=if-release',   # assert() off in release builds only
    'b_pie=true',
    'default_library=static',
  ],
)

cpp = meson.get_compiler('cpp')
cc_id = cpp.get_id()
host = host_machine.system()
is_msvc_like = cc_id in ['msvc', 'clang-cl']
buildtype = get_option('buildtype')
is_debug = buildtype in ['debug']
is_release = buildtype in ['release', 'minsize']

# ---------------------------------------------------------------------------
# Toolchain sanity check: the code uses <print> and <expected> (C++23)
# Needs GCC >= 14, Clang >= 18 (with a recent libstdc++/libc++), MSVC >= 19.37
# ---------------------------------------------------------------------------
foreach header : ['print', 'expected', 'format']
  if not cpp.has_header(header)
    error(
      'Missing C++23 header <@0@>. Use GCC >= 14, Clang >= 18 with a matching standard library, or MSVC 2022 >= 17.7.'.format(header),
    )
  endif
endforeach

# ---------------------------------------------------------------------------
# Warnings
# Flags unknown to the active compiler are dropped by get_supported_arguments()
# ---------------------------------------------------------------------------
warning_args = []
if is_msvc_like
  warning_args += [
    '/permissive-',
    '/utf-8',
    '/Zc:__cplusplus',
    '/Zc:inline',
    '/Zc:preprocessor',
    '/EHsc',
    '/w14242', '/w14254', '/w14263', '/w14265', '/w14287', '/we4289',
    '/w14296', '/w14311', '/w14545', '/w14546', '/w14547', '/w14549',
    '/w14555', '/w14619', '/w14640', '/w14826', '/w14905', '/w14906',
    '/w14928',
  ]
else
  warning_args += cpp.get_supported_arguments(
    '-Wshadow',
    '-Wconversion',
    '-Wsign-conversion',
    '-Wold-style-cast',
    '-Wcast-align',
    '-Wcast-qual',
    '-Wnon-virtual-dtor',
    '-Woverloaded-virtual',
    '-Wnull-dereference',
    '-Wdouble-promotion',
    '-Wformat=2',
    '-Wimplicit-fallthrough',
    '-Wmisleading-indentation',
    '-Wextra-semi',
    '-Wunused',
    '-Wundef',
    '-Wswitch-enum',
    '-Wpointer-arith',
    '-Wmissing-declarations',
    '-Wredundant-decls',
    # GCC only (silently skipped by Clang)
    '-Wduplicated-cond',
    '-Wduplicated-branches',
    '-Wlogical-op',
    '-Wuseless-cast',
    '-Wsuggest-override',
    # Clang only (silently skipped by GCC)
    '-Wcomma',
    '-Wloop-analysis',
    '-Wheader-hygiene',
    '-Wthread-safety',
  )
endif

# ---------------------------------------------------------------------------
# Platform defines
# ---------------------------------------------------------------------------
platform_args = []
if host == 'windows'
  platform_args += ['-DNOMINMAX', '-DWIN32_LEAN_AND_MEAN', '-DUNICODE', '-D_UNICODE']
endif

# ---------------------------------------------------------------------------
# Debug: sanitizers + standard library hardening
# Release: LTO + (optionally) -march=native
# ---------------------------------------------------------------------------
compile_args = []
link_args = []
sanitizers_opt = get_option('sanitizers')

if is_debug
  # --- standard library checks -------------------------------------------
  if is_msvc_like
    # MSVC debug CRT already enables iterator debugging (_ITERATOR_DEBUG_LEVEL=2)
    compile_args += ['/sdl', '/GS', '/guard:cf']
    link_args += ['/guard:cf']
  elif cpp.has_header_symbol('version', '_LIBCPP_VERSION')
    compile_args += ['-U_LIBCPP_HARDENING_MODE', '-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG']
  elif cpp.has_header_symbol('version', '__GLIBCXX__')
    compile_args += ['-D_GLIBCXX_DEBUG', '-D_GLIBCXX_DEBUG_PEDANTIC', '-D_GLIBCXX_ASSERTIONS']
  endif

  if not is_msvc_like
    compile_args += cpp.get_supported_arguments(
      '-fno-omit-frame-pointer',
      '-fstack-protector-strong',
      '-fstack-clash-protection',
      '-fcf-protection',
    )
  endif

  # --- sanitizers ----------------------------------------------------------
  want_san = not sanitizers_opt.disabled()
  san_flags = []
  if want_san
    if is_msvc_like
      san_flags = ['/fsanitize=address']             # MSVC: ASan only
    elif host == 'windows'
      # MinGW GCC/Clang ship no usable sanitizer runtimes.
      # (clang targeting the MSVC ABI is handled by the clang-cl branch.)
      san_flags = []
    elif host == 'linux'
      san_flags = ['-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow', '-fno-sanitize-recover=undefined']
    else
      san_flags = ['-fsanitize=address,undefined']
    endif

    if san_flags.length() > 0
      # Verify the toolchain can actually link the runtime before committing to it
      if cpp.links('int main() { return 0; }', args: san_flags, name: 'sanitizer runtime')
        compile_args += san_flags
        if not is_msvc_like
          link_args += san_flags
        endif
        message('Sanitizers enabled:', ' '.join(san_flags))
      elif sanitizers_opt.enabled()
        error('sanitizers=enabled but the toolchain cannot link: ' + ' '.join(san_flags))
      else
        warning('Sanitizers unavailable with this toolchain; continuing without them.')
      endif
    elif sanitizers_opt.enabled()
      error('sanitizers=enabled but none are supported on @0@ / @1@'.format(host, cc_id))
    endif
  endif
elif is_release
  # --- optimisation --------------------------------------------------------
  if is_msvc_like
    compile_args += ['/O2', '/Oi', '/Gy', '/GL', '/GS-', '/Zc:inline']
    link_args += ['/LTCG', '/OPT:REF', '/OPT:ICF']
  else
    compile_args += cpp.get_supported_arguments(
      '-O3',
      '-fno-plt',
      '-fvisibility=hidden',
      '-fvisibility-inlines-hidden',
      '-ffunction-sections',
      '-fdata-sections',
      # Cheap hardening that costs ~nothing
      '-D_FORTIFY_SOURCE=2',
      '-fstack-protector-strong',
    )
    if cc_id == 'clang'
      lto = '-flto=thin'
    else
      lto = '-flto=auto'
    endif
    if cpp.has_argument(lto) and cpp.links('int main() { return 0; }', args: [lto], name: 'LTO')
      compile_args += [lto]
      link_args += [lto]
    endif
    if host == 'linux'
      link_args += cpp.get_supported_link_arguments('-Wl,--gc-sections', '-Wl,-z,relro', '-Wl,-z,now')
    elif host == 'windows'
      link_args += cpp.get_supported_link_arguments('-Wl,--gc-sections')
    endif
  endif

  if get_option('native') and not is_msvc_like
    compile_args += cpp.get_supported_arguments('-march=native')
  elif get_option('native') and is_msvc_like
    compile_args += ['/arch:AVX2']
  endif
endif

add_project_arguments(warning_args, platform_args, compile_args, language: 'cpp')
add_project_link_arguments(link_args, language: 'cpp')

# ---------------------------------------------------------------------------
# Sources (listed explicitly; meson does not glob on purpose)
# ---------------------------------------------------------------------------
sources = files(
  'src/main.cpp',
  'src/login.cpp',
  'src/data/money.cpp',
  'src/util/io.cpp',
  # TODO (WIP): 'src/ui/...' once files exist
)

inc = include_directories('src')

atm_simulator = executable(
  'atm_simulator',
  sources,
  include_directories: inc,
  install: true,
  win_subsystem: 'console',
)

# ---------------------------------------------------------------------------
# Tests (WIP): enable once a tests/ directory exists
# ---------------------------------------------------------------------------
# subdir('tests')

summary(
  {
    'Compiler': cpp.get_id() + ' ' + cpp.version(),
    'Host': host,
    'Build type': buildtype,
    'Sanitizers': (is_debug and not sanitizers_opt.disabled()) ? 'requested (see log)' : 'off',
    'Native arch': get_option('native'),
    'Werror': get_option('werror'),
  },
  section: 'Configuration',
)
