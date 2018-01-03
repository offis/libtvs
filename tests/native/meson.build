
boost_system = dependency('boost', modules: ['system'])

native_files = files('producer.cpp', 'epoch.cpp')

native = executable('native_impl', 'native.cpp', native_files,
                    dependencies: [test_deps, boost_system])

test('native stream test', native)
