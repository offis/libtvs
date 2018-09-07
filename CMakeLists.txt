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

cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(TimedValueStreams VERSION 0.10.0 LANGUAGES CXX)

include(GNUInstallDirs)

# By default, build as shared library
option(BUILD_SHARED_LIBS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TVS_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

option(TVS_ENABLE_DOCS  "build documentation using Doxygen" OFF)
option(TVS_ENABLE_TESTS "build tests" ON)
option(TVS_USE_SYSTEMC  "use SystemC module hierarchy and data types" ON)

find_package(Boost 1.51.0 REQUIRED)

if(TVS_USE_SYSTEMC)
  find_package(SystemCLanguage 2.3.2)
else()
  message(STATUS "SystemC support disabled")
endif()

if((NOT SystemCLanguage_FOUND) AND TVS_USE_SYSTEMC)
  set(TVS_USE_SYSTEMC OFF)
  message(WARNING
    "SystemC dependency could not be found. Building without SystemC support.")
endif()

if(TVS_USE_SYSTEMC AND NOT(SystemC_CXX_STANDARD EQUAL CMAKE_CXX_STANDARD))
  message(FATAL_ERROR
    "This library needs C++14 support but the provided SystemC library was not "
    "built with the same C++ standard.")
endif()


add_subdirectory(src/tvs)

if(TVS_ENABLE_TESTS)
  enable_testing()

  find_package(GTest 1.8 QUIET)
  if(NOT GTEST_FOUND)
    message(STATUS "Building googletest via git submodule")
    execute_process(COMMAND
      git submodule update --init -- extern/googletest
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      )
    set(BUILD_GMOCK OFF)
    set(BUILD_GTEST ON)
    add_subdirectory(extern/googletest EXCLUDE_FROM_ALL)
    add_library(GTest::GTest ALIAS gtest)
  else()
    get_property(TVS_GTEST_LOCATION
      TARGET GTest::GTest
      PROPERTY LOCATION
      )
    message(STATUS "Found external gtest: ${TVS_GTEST_LOCATION}")
  endif()

  add_subdirectory(tests)
endif()

if(TVS_ENABLE_DOCS)
  find_package(Doxygen)

  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Documentation requested but doxygen was not found.")
  endif()

  configure_file(
    ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
    @ONLY
    )

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
    COMMENT "Generating documentation"
    VERBATIM
    )
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
  )

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${TVS_INSTALL_CONFIGDIR}
  )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${TVS_INSTALL_CONFIGDIR}
  )

# export the tvs library target to the build tree
export(TARGETS tvs
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE TVS::
  )

# export the whole target set to the installation tree
install(EXPORT tvs-targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    TVS::
  DESTINATION
    ${TVS_INSTALL_CONFIGDIR}
  )

# add this build tree to CMake's package registry
export(PACKAGE ${PROJECT_NAME})
