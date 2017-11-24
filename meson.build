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
        version : '0.10.0',
        meson_version: '>= 0.39',
        license : 'Apache 2.0')

# default directories
tvs_incdir  = get_option('includedir')
tvs_datadir = get_option('datadir')



project_deps  = []
project_deps += dependency('boost', version: '>=1.51')

project_incs = include_directories('src')

stdcheck_code = '''
#include "tvs/utils/cppver.h"
#if TVS_CPLUSPLUS < 201103L
#  error "Need at least C++11 support"
#endif
'''

if not meson.get_compiler('cpp').compiles(stdcheck_code, name : 'c++11 (or higher)',
                                          include_directories: project_incs)
  error('Need a compiler with C++11 (or higher) support.')
endif

sc_required = ''

if get_option('with_systemc')

  sc_version_required = '2.3.1'

  sc_libdir     = get_option('sc_libdir')
  sc_includedir = get_option('sc_includedir')

  # Check if SystemC path was given manually before trying pkg-config
  if sc_libdir != '' or sc_includedir != ''
    cc = meson.get_compiler('cpp')

    sc_includedir = include_directories(sc_includedir, is_system: true)
    if not cc.has_header('systemc', include_directories: sc_includedir)
      error('SystemC headers not found at @1@'.format())
    endif

    sc_lib = cc.find_library('systemc', dirs: sc_libdir)
    if not sc_lib.found()
      error('SystemC library not found')
    endif

    sc_vercheck_code = '''
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

  extern "C" int
  sc_main(int argc, char* argv[]) { return 0; }
  '''

    sc_ver = sc_version_required.split('.')
    if not cc.links(sc_vercheck_code.format(sc_ver[0], sc_ver[1], sc_ver[2]),
                    dependencies: sc_lib, include_directories: sc_includedir)
      error('Cannot link against SystemC >= @0@ at @1@'.format(sc_version_required,
                                                      sc_libdir))
    endif

    systemc_dep = declare_dependency(dependencies: sc_lib,
                                    include_directories: sc_includedir)
    message('Using manually specified SystemC at @0@'.format(sc_libdir))
  else
    # Check using pkg-config
    systemc_dep = dependency('systemc',
                            version: '>= @0@'.format(sc_version_required))
    message('SystemC @0@ found using pkg-config'.format(systemc_dep.version()))
  endif

  project_deps += systemc_dep

  sc_required = 'systemc >= @0@'.format(sc_version_required)
else
  add_project_arguments('-DSYSX_NO_SYSTEMC', language: 'cpp')
endif # with_systemc

# the library tracing_lib is defined in this subdirectory
subdir('src/tvs')

# This variable can be queried by parent projects to use the library as a
# dependency
libtvs_dep = declare_dependency(link_with: tracing_lib,
                                include_directories: project_incs,
                                dependencies: project_deps)

pkg = import('pkgconfig')
pkg.generate(name: 'tvs',
             description: meson.project_name(),
             url: 'http://www.github.com/offis/libtvs',
             libraries : tracing_lib,
             version : meson.project_version(),
             requires_private: sc_required)


if get_option('enable_tests')
  subdir('tests')
endif

if get_option('enable_docs')
  doxygen = find_program('doxygen', required: false)
  if doxygen.found()
    subdir('doc')
  else
    message('Documentation disabled without doxygen')
  endif
endif

