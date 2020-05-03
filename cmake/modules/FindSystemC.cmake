
# Try to find the package using CMake Config files.

# First, try to get the SystemC package via a SystemCLanguageConfig.cmake from
# an available installation.
find_package(SystemCLanguage ${PACKAGE_FIND_VERSION} QUIET CONFIG)


if(SystemCLanguage_FOUND)
  message(STATUS "Found SystemC ${SystemCLanguage_VERSION} via ${SystemCLanguage_CONFIG}")
else()

  message(STATUS "Looking for SystemC in system directories and via SYSTEMC_HOME")

  # The manual approach

  # TODO: version check

  find_path (SYSTEMC_INCLUDE_DIR systemc
    PATH_SUFFIXES include
    HINTS ENV SYSTEMC_HOME
    )

  find_library (SYSTEMC_LIBRARY systemc
    PATH_SUFFIXES lib64 lib lib-linux64
    HINTS ENV SYSTEMC_HOME
    )

  set(SYSTEMC_INCLUDE_DIRS ${SYSTEMC_INCLUDE_DIR})

  set(SYSTEMC_LIBRARIES ${SYSTEMC_LIBRARY})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SystemC
    "Cannot find SystemC. Consider setting the environment variable SYSTEMC_HOME, or alternatively provide the correct paths via the CMake options SYSTEMC_LIBRARY and SYSTEMC_INCLUDE_DIR."
    SYSTEMC_LIBRARY SYSTEMC_INCLUDE_DIR)
  mark_as_advanced(SYSTEMC_INCLUDE_DIR SYSTEMC_LIBRARY)

  add_library(SystemC::systemc INTERFACE IMPORTED GLOBAL)

  target_link_libraries(SystemC::systemc INTERFACE ${SYSTEMC_LIBRARIES})

  set_property(TARGET SystemC::systemc PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES
    ${SYSTEMC_INCLUDE_DIRS})

  message(STATUS "Found SystemC at ${SYSTEMC_LIBRARY} (header at ${SYSTEMC_INCLUDE_DIR})" )

endif()
