#
# Copyright (c) 2018 OFFIS Institute for Information Technology
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

macro(package_add_test TESTNAME)
  add_executable(${TESTNAME} ${ARGN} common/gtest_scmain.cpp)
  target_link_libraries(${TESTNAME}
    PRIVATE
    GTest::GTest
    TVS::tvs
    )
  add_test(${TESTNAME} ${TESTNAME})
endmacro()


package_add_test(StateSemantics        tv_streams_state_semantics.cpp)
package_add_test(ProcessSemantics      tv_streams_process_semantics.cpp)
package_add_test(EventSemantics        tv_streams_event_semantics.cpp)
package_add_test(CustomTraitsSemantics tv_streams_custom_traits.cpp)
package_add_test(SequenceSemantics     tv_streams_sequence_semantics.cpp)


if(TVS_USE_SYSTEMC)
  package_add_test(VCDTestbench stream_processing_test/main.cpp)
endif()
