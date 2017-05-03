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

cdata = configuration_data()

cdata.set('PROJECT_NAME',    meson.project_name())
cdata.set('PROJECT_VERSION', meson.project_version())

cdata.set('OUTPUT_DIR',      meson.current_build_dir())

cdata.set('INPUT_DIR', join_paths(meson.source_root(), 'src/tvs'))

if find_program('dot', required: false).found()
  cdata.set('HAVE_DOT', 'YES')
endif


doxyfile = configure_file(input: 'Doxyfile.in',
                          output: 'Doxyfile',
                          configuration: cdata,
                          install: false
                         )

datadir = join_paths(tvs_datadir, 'doc', 'tvs')

docs = custom_target('tvs-docs',
                     input: doxyfile,
                     output: 'html',
                     install: true,
                     install_dir: datadir,
                     command: [doxygen, doxyfile])

