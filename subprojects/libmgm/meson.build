project('libmgm', ['cpp'], default_options: ['cpp_std=c++17'])

sources =  [
  'libmgm/details/io_utils.cpp',
  'libmgm/details/multigraph.cpp',
  'libmgm/details/costs.cpp',
  'libmgm/details/lap_interface.cpp',
  'libmgm/details/logger.cpp',
  'libmgm/details/solution.cpp',
  'libmgm/details/qap_interface.cpp',
  'libmgm/details/cliques.cpp',
  'libmgm/details/solver_generator_mgm.cpp',
  'libmgm/details/solver_local_search_GM.cpp',
  'libmgm/details/solver_local_search_swap.cpp',
  'libmgm/details/solver_generator_incremental.cpp',
  'libmgm/details/synchronization.cpp'
]

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

json_proj  = subproject('nlohmann_json')
json_dep   = json_proj.get_variable('nlohmann_json_dep')

unordered_dense  = subproject('unordered_dense')
unordered_dense_dep   = unordered_dense.get_variable('unordered_dense_dep')

spdlog_proj  = subproject('spdlog', default_options: ['external_fmt=disabled'])
spdlog_dep   = spdlog_proj.get_variable('spdlog_dep')

lsap_proj  = subproject('scipy_lsap')
lsap_dep   = lsap_proj.get_variable('liblsap_dep')

openmp = dependency('openmp', required: false)
if not openmp.found()
  cpp = meson.get_compiler('cpp')
  openmp = cpp.find_library('libomp', dirs: '/usr/local/lib', required: true) # potential custom install location
endif

# Windows needs additional flag for newer OpenMP version.
if meson.get_compiler('cpp').get_id() == 'msvc'
  add_project_arguments('-openmp:llvm', language : 'cpp')
endif

include_dirs = include_directories('.')

deps = [libqpbo_dep, libmpopt_dep, spdlog_dep, json_dep, openmp, unordered_dense_dep, lsap_dep]
libmgm = static_library(
                    'libmgm', 
                    sources,
                    include_directories: include_dirs,
                    dependencies: deps,
                    install: false)

libmgm_dep = declare_dependency(include_directories : include_dirs, link_with : libmgm, dependencies: deps)
