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

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

json_proj  = subproject('nlohmann_json')
json_dep   = json_proj.get_variable('nlohmann_json_dep')

unordered_dense  = subproject('unordered_dense')
unordered_dense_dep   = unordered_dense.get_variable('unordered_dense_dep')

cli11_proj  = subproject('cli11')
cli11_dep   = cli11_proj.get_variable('CLI11_dep')

spdlog_proj  = subproject('spdlog')
spdlog_dep   = spdlog_proj.get_variable('spdlog_dep', 'external_fmt=false')

executable('mgm',
  sources,
  dependencies : [libmgm_dep, libqpbo_dep, libmpopt_dep, spdlog_dep, json_dep, cli11_dep, unordered_dense_dep])

python_sources =  [
  'mgm_python/libmgm_python.cpp'
]

pybind11_proj = subproject('pybind11')
pybind11_dep  = pybind11_proj.get_variable('pybind11_dep')

py = import('python').find_installation('python3')
python3_dep = py.dependency()

message('Using python interpreter path:', py.path())
message('Installing python package to:', py.get_install_dir())

py.extension_module('pylibmgm', 
                    sources : python_sources, 
                    dependencies : [libmgm_dep, libqpbo_dep, libmpopt_dep, spdlog_dep, json_dep, cli11_dep, unordered_dense_dep, pybind11_dep, python3_dep],
                    install: true,
                    install_dir: py.get_install_dir())