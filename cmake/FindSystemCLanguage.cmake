
# Try to find the package using CMake Config files.

#find_package(SystemCLanguage ${PACKAGE_FIND_VERISON} CONFIG QUIET)

if (NOT SystemCLanguage_FOUND)

  set(_SYSC_HINTS
    ${SYSTEMC_HOME}/include
    ${SYSTEMC_HOME}/lib
    $ENV{SYSTEMC_}/lib-linux64
    $ENV{SYSTEMC_}/include)


  find_path(SYSTEMC_INCLUDE_DIR
    NAMES systemc systemc.h
    HINTS ${_SYSC_HINTS}
    )

  find_library(SYSTEMC_LIBRARY
    NAMES systemc SystemC
    HINTS ${_SYSC_HINTS}
    )

  get_filename_component(SYSTEMC_LIBRARY_DIR ${SYSTEMC_LIBRARY} DIRECTORY)

  include(FindPackageHandleStandardArgs)

  find_package_handle_standard_args(SYSTEMC DEFAULT_MSG
    SYSTEMC_LIBRARY SYSTEMC_INCLUDE_DIR)

  mark_as_advanced(SYSTEMC_INCLUDE_DIR SYSTEMC_LIBRARY)

  set(SYSTEMC_LIBRARIES ${SYSTEMC_LIBRARY})
  set(SYSTEMC_INCLUDE_DIRS ${SYSTEMC_INCLUDE_DIR})

  add_library(SystemC::systemc INTERFACE IMPORTED GLOBAL)

  target_link_libraries(SystemC::systemc INTERFACE ${SYSTEMC_LIBRARIES})

  set_property(TARGET SystemC::systemc PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES
    ${SYSTEMC_INCLUDE_DIRS})


endif()
