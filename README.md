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

To build the library, create a build directory and run ``cmake`` in the new
directory while pointing it to the source directory.  Assuming you have
cloned/extracted the repository in ``$PWD``, you can use the following commands
to generate the build files:

    $ mkdir -p build
    $ cd build
    $ cmake ..

After that, the library can be built using:

    $ cmake --build .

## Build Options

You can use ``cmake -L`` in the build directory to get a list of all build
settings.  Another way to browse and change the settings in an existing build
directory is ``ccmake``.

Build options can be set when creating the build system, e.g.:

    $ cmake .. -D<var>=<value>

Refer to the CMake documentation ``cmake(1)`` for a more detailed description on
how to set options.

## SystemC Dependency

If a SystemC build is requested (by default), the CMake will search for a
suitable SystemC library via the internal ``find_package`` mechanism.
Therefore, SystemC needs to be installed with ``cmake`` support, which is
available starting at SystemC version 2.3.2.  See `cmake-packages(7)` for more
information.

Building with SystemC support enables the Timed-Value Streams to be integrated
into the the SystemC module hierarchy.  Furthermore, the library will use the
SystemC time type as its default unit.  Streams will be created as ``sc_object``
instances within the module hierarchy.  The interface of a ``timed_value`` will
internally use the ``sc_core::sc_time`` datatype for time and duration storage
and provide conversion operators for ``boost::units``.

If built without SystemC support, the library will use the `boost::units`
datatypes for representing time durations and it will use its own, minimal
object registry for supporting name-based stream binding.

## Test Suite and Documentation

By default, a test suite is built which contains unit tests and example files.
The unit tests require the Google Test framework ``gtest``.  If `gtest` is not
available on your system, the build system will automatically download and
compile the dependency.  The build option `TVS_ENABLE_TESTS` controls if the
test suite should be built.

``TVS_ENABLE_DOCS`` enables generating the preliminary documentation using
Doxygen.
