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

add_library(tvs

  units/common_impl.cpp

  utils/report/message.cpp
  utils/report/report_base.cpp
  utils/variant.cpp
  utils/variant_traits.cpp

  tracing/timed_annotation.cpp
  tracing/timed_duration.cpp
  tracing/timed_object.cpp
  tracing/timed_reader_base.cpp
  tracing/timed_stream_base.cpp
  tracing/timed_writer_base.cpp
  tracing/processors/timed_stream_processor_base.cpp
  tracing/processors/timed_stream_vcd_processor.cpp
  )

add_library(TVS::tvs ALIAS tvs)

set_target_properties(tvs
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

target_include_directories(tvs
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
  )

target_compile_definitions(tvs
  PUBLIC
  $<$<NOT:$<BOOL:${TVS_USE_SYSTEMC}>>:SYSX_NO_SYSTEMC>
  )


target_link_libraries(tvs
  PUBLIC
    $<$<BOOL:${TVS_USE_SYSTEMC}>:SystemC::systemc>
    Boost::boost
  )

install(TARGETS tvs
  EXPORT tvs-targets
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )


install(DIRECTORY   ./
        DESTINATION include/tvs
          FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.tpp"
            )
