project('multi-matching-optimization', ['c', 'cpp'], default_options: ['cpp_std=c++17'])

sources =  [
  'test.cpp'
]

libqpbo_proj  = subproject('libqpbo')
libqpbo_dep   = libqpbo_proj.get_variable('libqpbo_dep')

libmpopt_proj  = subproject('libmpopt')
libmpopt_dep   = libmpopt_proj.get_variable('libmpopt_dep')

executable('my_test',
  sources,
  dependencies : [libqpbo_dep, libmpopt_dep])