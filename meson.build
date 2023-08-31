project('multi-matching-optimization', ['cpp'], default_options: ['cpp_std=c++17', 'buildtype=debugoptimized', 'warning_level=3'])

sources =  [
  'main.cpp'
]

libmgm_proj  = subproject('libmgm')
libmgm_dep   = libmgm_proj.get_variable('libmgm_dep')

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

json_proj  = subproject('nlohmann_json')
json_dep   = json_proj.get_variable('nlohmann_json_dep')

spdlog = dependency('spdlog')
fmt = dependency('fmt')

executable('mgm',
  sources,
  dependencies : [libmgm_dep, libqpbo_dep, libmpopt_dep, spdlog, fmt, json_dep])