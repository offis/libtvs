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


gtest = dependency('gtest', main: false, required: false)

# unfortunately, gtest wrap file does not declare a dependency
if not gtest.found()
  gt_sub = subproject('gtest')
  gtest = declare_dependency(include_directories : gt_sub.get_variable('incdir'),
                             sources : gt_sub.get_variable('libsources'),
                             dependencies : dependency('threads'))
endif


tests = [
  ['State Semantics',         'state',         'tv_streams_state_semantics.cpp'],
  ['Process Semantics',       'process',       'tv_streams_process_semantics.cpp'],
  ['Custom Traits Semantics', 'custom_traits', 'tv_streams_custom_traits.cpp'],
  ['Sequence Semantics',      'sequence',      'tv_streams_sequence_semantics.cpp'],
  ['VCD Testbench',           'vcd_testbench', 'stream_processing_test/main.cpp'],
  ]

incs = include_directories('../src')
deps = [ gtest, systemc_dep ]

extra_srcs = 'common/gtest_scmain.cpp'

foreach test : tests
  test(test[0],
       executable(test[1], test[2], extra_srcs,
                  include_directories: incs,
                  link_with: tracing_lib,
                  dependencies: deps))
endforeach

