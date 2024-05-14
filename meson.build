project('multi-matching-optimization', 
        ['cpp'], 
        default_options: 
          ['cpp_std=c++17', 
          'buildtype=debugoptimized', 
          'warning_level=3'])

sources =  [
  'mgm/main.cpp',
  'mgm/argparser.cpp',
  'mgm/runner.cpp'
]

libmgm_proj  = subproject('libmgm')
libmgm_dep   = libmgm_proj.get_variable('libmgm_dep')

cli11_proj  = subproject('cli11')
cli11_dep   = cli11_proj.get_variable('CLI11_dep')

spdlog_proj  = subproject('spdlog')
spdlog_dep   = spdlog_proj.get_variable('spdlog_dep', 'external_fmt=false')

executable('mgm',
  sources,
  dependencies : [libmgm_dep, spdlog_dep, cli11_dep])

python_sources =  [
  'mgm_python/libmgm_python.cpp'
]

pybind11_proj = subproject('pybind11')
pybind11_dep  = pybind11_proj.get_variable('pybind11_dep')

py = import('python').find_installation('python3')
python3_dep = py.dependency()

message('Using python interpreter path:', py.path())
message('Installing python package to:', py.get_install_dir())


py.install_sources(
  ['mgm_python/solver.py', 'mgm_python/__init__.py'],
  subdir: 'pylibmgm')

target_dir = join_paths(py.get_install_dir(), 'pylibmgm')
py.extension_module('_pylibmgm', 
                    sources : python_sources, 
                    dependencies : [libmgm_dep, pybind11_dep, python3_dep],
                    install: true,
                    install_dir: target_dir
                    )

stubgen = find_program('stubgen', required: false)
if stubgen.found()
    result = run_command(stubgen, '-o', py.get_install_dir(), '-m', 'pylibmgm._pylibmgm', '--search-path', target_dir, check: true)
    message(result.stdout().strip())
    message(result.stderr().strip())
else
    message('Did not find stubgen. Please install python package mypy into virtual environment, to get python stubs (autocomplete hints) for this package.')
endif