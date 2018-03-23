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

The library requires the build system ``meson >= 0.39``.  By default, meson uses
the `ninja` backend to generate the build instructions.  On debian-based
systems, you can install ninja via

    $ sudo apt-get install ninja-build

If ``meson`` is not available via your distribution package manager, you can
install it via `pip` (specifically, the python3 `pip` variant):

    $ pip3 install meson
    
Alternatively, you can download the archive or check out the sources from
https://github.org/mesonbuild/meson and use `meson.py` directly.  Refer to the
`meson` website at http://mesonbuild.com/ for more information.

### Library Dependencies

The library requires a compiler with C++11 support.  It was successfully tested
with ``gcc 6.3`` and ``clang 6.0``.
  
External dependencies:

- `SystemC >= 2.3.2`  (optional, default=yes)
- `boost >= 1.51`

### Building the Library

To build the library, create a build directory and run `meson` in the new
directory while pointing it to the source directory:

    $ cd <path/to/libtvs>
    $ mkdir objdir
    $ cd objdir
    $ meson ..

After configuring the build directory, you can run the build using ``ninja``.
``ninja test`` will run the test suite.
    
## Build Options

The library-specific build options can be queried using ``meson configure``.

You can either directly specify the build options when configuring the build
directory using ``meson -D<name>=<value> <...>`` or use ``mesonconf
-D<name>=<value>`` after having configured the directory.  You can also use
``mesonconf`` to inspect all available build configuration options.

## SystemC Dependency
    
If a SystemC build is requested, the build system will search for a suitable
SystemC library via `pkg-config`.  If a `pkg-config` file is not available, you
can optionally specify custom SystemC library and include header paths via the
options `-Dsc_includedir` and `-Dsc_libdir`:

    $ mkdir objdir
    $ cd objdir
    $ meson -Dsc_libdir=/opt/systemc-2.3.1/lib-linux64 \
          -Dsc_includedir=/opt/systemc-2.3.1/include ..
          
Building with SystemC support enables re-use of the SystemC module hierarchy and
data types.  Streams will be created as ``sc_object`` instances within the
module hierarchy.  The interface of a ``timed_value`` will also internally use
the ``sc_core::sc_time`` datatype for time and duration storage and provide
conversion operators for ``boost::units``.

## Test Suite and Documentation
          
By default, a test suite is built which contains unit tests and example files.
The unit tests require the Google Test framework ``gtest``.  If `gtest` is not
available on your system, `meson` will download and compile the dependency
automatically.  The build option `enable_tests` controls if the test suite
should be built.

To enable generating the preliminary documentation using Doxygen, set
``enable_docs`` to true:

    $ mesonconf -Denable_docs=true
    $ ninja

