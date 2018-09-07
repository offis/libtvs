# Timed-Value Streams

[![Travis](https://travis-ci.org/offis/libtvs.svg?branch=develop)](https://travis-ci.org/offis/libtvs)

The *Timed Value Stream Library* provides an advanced framework for
instrumentation, pre-processing and recording of functional and extra-functional
properties in SystemC-based virtual prototyping simulations.

This repository contains an implementation of the library described in the paper
*Advanced SystemC Tracing and Analysis Framework for Extra-Functional
Properties*, Philipp A. Hartmann, Kim Grüttner, and Wolfgang Nebel, Procceedings
of the 11th International Symposium on Applied Reconfigurable Computing
(ARC'15).

## Abstract

System-level simulations are an important part in the design flow for today's
complex systems-on-a-chip.  Trade-off analysis during architectural exploration
as well as run-time reconfiguration of applications and their mapping require
detailed introspection of the dynamic effects on the target platform.
Additionally, extra-functional properties like power consumption and performance
characteristics are important metrics to assess the quality of a design.  In
this paper, we present an advanced framework for instrumentation, pre-processing
and recording of functional and extra-functional properties in SystemC-based
virtual prototyping simulations.  The framework is based on a hierarchy of
so-called timed value streams, allowing to address the requirements for highly
configurable, dynamic architectures while allowing tailored introspection of the
required system characteristics under analysis.

The full paper is available at
http://link.springer.com/chapter/10.1007/978-3-319-16214-0_12

## Build System and Dependencies

The library requires the build system ``CMake >= 3.7``.

### Library Dependencies

The library requires a compiler with C++11 support. It was successfully tested
with the following toolchains:

- ``gcc-5``, ``gcc-6``, ``gcc-7``
- ``clang-3.8``, ``clang-5.0``, ``clang-6.0``
  
External dependencies:

- `SystemC >= 2.3.2`  (optional, default=yes)
- `boost >= 1.51`

### Building the Library

To build the library, create a build directory and run `meson` in the new
directory while pointing it to the source directory:

    $ cmake <path/to/libtvs> -Bbuild
    $ cd build
    $ make

## Build Options

The library-specific build options can be queried using ``meson configure``.

You can either directly specify the build options when configuring the build
directory using ``cmake .. -D<name>=<value>`` or ``ccmake <objdir>`` for a
graphical configuration screen.

## SystemC Dependency

If a SystemC build is requested, the build system will search for a suitable
SystemC library via the internal ``find_package`` mechanism.  Therefore, SystemC
needs to be installed with ``cmake`` support, which is available starting at
SystemC version 2.3.2.  See `cmake-packages(7)` for more information.

Building with SystemC support enables Timed-Value Streams to be integrated into
the the SystemC module hierarchy and data types.  Streams will be created as
``sc_object`` instances within the module hierarchy.  The interface of a
``timed_value`` will also internally use the ``sc_core::sc_time`` datatype for
time and duration storage and provide conversion operators for ``boost::units``.

## Test Suite and Documentation
          
By default, a test suite is built which contains unit tests and example files.
The unit tests require the Google Test framework ``gtest``.  If `gtest` is not
available on your system, The build system will automatically download and
compile the dependency.  The build option `TVS_ENABLE_TESTS` controls if the
test suite should be built.

``TVS_ENABLE_DOCS`` enables generating the preliminary documentation using
Doxygen.


