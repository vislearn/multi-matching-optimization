project('multi-matching-optimization', 
        ['cpp'], 
        default_options: 
          ['cpp_std=c++17', 
          'buildtype=debugoptimized', 
          'warning_level=3'])

#
# C++ executable
#
sources =  [
  'mgm/main.cpp',
  'mgm/argparser.cpp',
  'mgm/runner.cpp'
]

libmgm_proj  = subproject('libmgm')
libmgm_dep   = libmgm_proj.get_variable('libmgm_dep')

cli11_proj  = subproject('cli11')
cli11_dep   = cli11_proj.get_variable('CLI11_dep')

spdlog_proj  = subproject('spdlog', default_options: ['external_fmt=disabled'])
spdlog_dep   = spdlog_proj.get_variable('spdlog_dep')

if not get_option('pypackage')
    # Build C++ executable
    executable('mgm',
    sources,
    dependencies : [libmgm_dep, spdlog_dep, cli11_dep])
else
    # Build python extension
    pybind11_proj = subproject('pybind11')
    pybind11_dep  = pybind11_proj.get_variable('pybind11_dep')

    py = import('python').find_installation('python3', pure: false)
    python3_dep = py.dependency()

    message('Using python interpreter path:', py.full_path())
    message('Installing python package to:', py.get_install_dir())

    py.install_sources(
    ['mgm_python/solver.py', 'mgm_python/__init__.py'],
    subdir: 'pylibmgm')

    package_dir = join_paths(py.get_install_dir(), 'pylibmgm')
    py.extension_module('_pylibmgm', 
                        sources : ['mgm_python/libmgm_python.cpp', 'mgm_python/logging_adapter.cpp'], 
                        dependencies : [libmgm_dep, pybind11_dep, python3_dep],
                        install: true,
                        install_dir: package_dir
                        )

    py.extension_module('io', 
                        sources : ['mgm_python/libmgm_io_python.cpp', 'mgm_python/logging_adapter.cpp'], 
                        dependencies : [libmgm_dep, pybind11_dep, python3_dep],
                        install: true,
                        install_dir: package_dir
                        )

    py.install_sources(
        ['mgm_python/stubs/pylibmgm/__init__.pyi', 
         'mgm_python/stubs/pylibmgm/io.pyi', 
         'mgm_python/stubs/pylibmgm/solver.pyi'], 
        subdir: 'pylibmgm'
    )
endif