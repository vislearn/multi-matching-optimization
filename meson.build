project('multi-matching-optimization', ['cpp'], default_options: ['cpp_std=c++17', 'optimization=0', 'buildtype=debug'])

sources =  [
  'libmgm/main.cpp',
  'libmgm/argparser.cpp',
  'libmgm/dd_parser.cpp',
  'libmgm/multigraph.cpp',
  'libmgm/costs.cpp',
  'libmgm/solution.cpp',
  'libmgm/qap_interface.cpp',
  'libmgm/cliques.cpp'
]

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
  dependencies : [libqpbo_dep, libmpopt_dep, spdlog, fmt, json_dep])