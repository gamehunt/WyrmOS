includes = include_directories('include')

srcs = [
  'list.c',
  'tree.c',
  'util.c'
]

ktypes = static_library('ktypes', 
  sources: srcs,
  include_directories: [includes],
  c_args: [
    c_args_freestanding,
  ],
  dependencies: [libk_dep],
  install: true,
  install_dir: 'lib'
)
install_subdir('include', install_dir: 'include', strip_directory: true)

ktypes_dep = declare_dependency(link_with : ktypes, include_directories : includes)
