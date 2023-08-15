project('multi-matching-optimization', ['c', 'cpp'], default_options: ['cpp_std=c++17', 'buildtype=release'])

sources =  [
  'libmgm/main.cpp',
  'libmgm/argparser.cpp',
  'libmgm/dd_parser.cpp',
  'libmgm/multigraph.cpp',
  'libmgm/datastructures.cpp',
  'libmgm/solution.cpp',
  'libmgm/qap_interface.cpp'
]

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

json_proj  = subproject('nlohmann_json')
json_dep   = json_proj.get_variable('nlohmann_json_dep')

spdlog = dependency('spdlog')

executable('mgm',
  sources,
  dependencies : [libqpbo_dep, libmpopt_dep, spdlog, json_dep])