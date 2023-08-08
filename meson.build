project('multi-matching-optimization', ['c', 'cpp'], default_options: ['cpp_std=c++17'])

sources =  [
  'libmgm/main.cpp',
  'libmgm/argparser.cpp'
]

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

spdlog = dependency('spdlog')

executable('mgm',
  sources,
  dependencies : [libqpbo_dep, libmpopt_dep, spdlog])