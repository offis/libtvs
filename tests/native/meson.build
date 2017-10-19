
boost_system = dependency('boost', modules: ['system'])

native = executable('native_impl', 'producer.cpp', 'native.cpp', include_directories: incs,
                    link_with: tracing_lib,
                    dependencies: [deps, boost_system])
