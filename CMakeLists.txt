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

# Install libraries with the rpath settings to avoid requiring LD_LIBRARY_PATH
# to be set correctly.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# always create PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake/modules/")


# By default, build as shared library
option(BUILD_SHARED_LIBS "build shared libraries" ON)

set(TVS_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

option(TVS_ENABLE_DOCS  "build documentation using Doxygen" OFF)
option(TVS_ENABLE_TESTS "build tests" ON)
option(TVS_USE_SYSTEMC  "use SystemC module hierarchy and data types" ON)

# the minimum C++ standard
set(TVS_MIN_CXX_STANDARD 14)
mark_as_advanced(TVS_MIN_CXX_STANDARD)

# the default C++ standard; will be adjusted depending on SystemC info
set(TVS_CXX_STANDARD 14 CACHE STRING "The default C++ standard for building")


find_package(Boost 1.51.0 REQUIRED)

if(TVS_USE_SYSTEMC)

  # use FindSystemC.cmake from cmake/modules/
  find_package(SystemC 2.3.2 REQUIRED MODULE)

  # Handle SystemC C++ standard
  if(NOT DEFINED SystemC_CXX_STANDARD)
    message(STATUS "Cannot determine SystemC C++ standard, consider adjusting TVS_CXX_STANDARD if linking fails")
  elseif(SystemC_CXX_STANDARD GREATER TVS_CXX_STANDARD)
    message(STATUS "SystemC was built with C++${SystemC_CXX_STANDARD}, setting C++ standard accordingly")
    set(TVS_CXX_STANDARD ${SystemC_CXX_STANDARD})
  endif()
else()
  message(STATUS "SystemC support disabled")
endif()

set(CMAKE_CXX_STANDARD ${TVS_CXX_STANDARD})

if(CMAKE_CXX_STANDARD LESS ${TVS_MIN_CXX_STANDARD})
  message(FATAL_ERROR
    "This Library requires at least C++14, but was configured to use C++${TVS_CXX_STANDARD}")
endif()

set(CMAKE_CXX_EXTENSIONS OFF) # don't use gnu extensions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Using C++${CMAKE_CXX_STANDARD}")

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


add_subdirectory(src)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
  )

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${TVS_INSTALL_CONFIGDIR}
  )


install(DIRECTORY include/
  DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
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
