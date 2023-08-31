project('libmgm', ['cpp'], default_options: ['cpp_std=c++17'])

sources =  [
  'libmgm/details/argparser.cpp',
  'libmgm/details/dd_parser.cpp',
  'libmgm/details/multigraph.cpp',
  'libmgm/details/costs.cpp',
  'libmgm/details/solution.cpp',
  'libmgm/details/qap_interface.cpp',
  'libmgm/details/cliques.cpp',
  'libmgm/details/solver_mgm.cpp'
]

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

json_proj  = subproject('nlohmann_json')
json_dep   = json_proj.get_variable('nlohmann_json_dep')

spdlog = dependency('spdlog')
fmt = dependency('fmt')

include_dirs = include_directories('.')

libmgm = static_library(
                    'libmgm', 
                    sources,
                    include_directories: include_dirs,
                    dependencies: [libqpbo_dep, libmpopt_dep, spdlog, fmt, json_dep],
                    install: false)

libmgm_dep = declare_dependency(include_directories : include_dirs, link_with : libmgm)