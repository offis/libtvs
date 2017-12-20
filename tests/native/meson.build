
boost_system = dependency('boost', modules: ['system'])

native = executable('native_impl', 'producer.cpp', 'native.cpp', 'epoch.cpp',
                    dependencies: [test_deps, boost_system])
