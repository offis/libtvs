#
# Copyright (c) 2017 OFFIS Institute for Information Technology
#                          Oldenburg, Germany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project('Timed-Value Streams', 'cpp',
        version : '0.9.0',
        meson_version: '>= 0.39',
        license : 'Apache 2.0')

# default directories
tvs_incdir  = get_option('includedir')
tvs_datadir = get_option('datadir')

code = '''
#if __cplusplus < 201103L
#error "Need at least C++11 support"
#endif
'''

if not meson.get_compiler('cpp').compiles(code, name : 'c++11 (or higher)')
  error('Need a compiler with C++11 (or higher) support.')
endif


proj_deps = []

sc_version_required = '2.3.1'

# Check using pkg-config
systemc_dep = dependency('systemc',
                          version: '>= @0@'.format(sc_version_required),
                          required: false)

# fall back to SystemC given via sc_includedir and sc_libdir options
if not systemc_dep.found()

  sc_includedir = include_directories(get_option('sc_includedir'), is_system: true)
  cc = meson.get_compiler('cpp')
  if not cc.has_header('systemc', include_directories: sc_includedir)
    error('SystemC headers not found at sc_includedir')
  endif

  sc_libdir = get_option('sc_libdir')
  sc_lib = cc.find_library('systemc', dirs: sc_libdir)
  if not sc_lib.found()
    error('SystemC library not found at sc_libdir')
  endif

  systemc_dep = declare_dependency(dependencies: sc_lib,
                                    include_directories: sc_includedir)

  # Manually check the SystemC version
  sc_ver = sc_version_required.split('.')
  if not cc.compiles('''
#include <systemc>
#define _MAKE_VERSION(x,y,z) (((x)*1000000) + ((y)*1000) + (z))
#if defined(IEEE_1666_SYSTEMC) && IEEE_1666_SYSTEMC >= 201101L
#  define __SCVER _MAKE_VERSION(SC_VERSION_MAJOR,SC_VERSION_MINOR,SC_VERSION_PATCH)
#elif defined(SYSTEMC_VERSION) // pre-1666-2011
#  if SYSTEMC_VERSION == 20050714 // SystemC 2.1.v1
#    define __SCVER _MAKE_VERSION(2,1,1)
#  elif SYSTEMC_VERSION == 20060505 // SystemC kernel 2.2.05jun06_beta
#    define __SCVER _MAKE_VERSION(2,1,90)
#  elif SYSTEMC_VERSION == 20070314 // SystemC kernel 2.2.0
#    define __SCVER _MAKE_VERSION(2,2,0)
#  elif SYSTEMC_VERSION == 20120610 // SystemC 2.3.0_pub_rev_20120610
#    define __SCVER _MAKE_VERSION(2,2,90)
#  endif
#else // !1666-2011 and no SYSTEMC_VERSION
#  error "Cannot determine SystemC version."
#endif // !1666-2011
#if __SCVER < _MAKE_VERSION(@0@, @1@, @2@)
# error "SystemC version check failed."
#endif
'''.format(sc_ver[0], sc_ver[1], sc_ver[2]), dependencies: sc_lib, include_directories: sc_includedir)
    error('Cannot find SystemC >= @0@ at @1@'.format(sc_version_required, sc_libdir))
  endif
endif

proj_deps += systemc_dep

proj_deps += dependency('boost', version: '>=1.51')

proj_includes = include_directories('src')

subdir('src/tvs')

if get_option('enable_tests')
  subdir('tests')
endif

# Generate pkg-config file for this library
pkg = import('pkgconfig')
pkg.generate(name: 'tvs',
             description: meson.project_name(),
             libraries : tracing_lib,
             version : meson.project_version(),
             libraries_private: '-lsystemc')


if get_option('enable_docs')
  doxygen = find_program('doxygen', required: false)
  if doxygen.found()
    subdir('doc')
  else
    message('Documentation disabled without doxygen')
  endif
endif

