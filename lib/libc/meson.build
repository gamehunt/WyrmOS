includes = include_directories('include')

srcs = [
  'ctype/ctype.c',
  'string/memory.c',
  'string/string.c',
  'stdio/printf.c',
  'stdlib/stdlib.c'
]

# library('libc', 'source.c')
libk = static_library('k', 
  sources: srcs,
  include_directories: [includes, kernel_includes],
  c_args: [
    c_args_freestanding,
    '-D__LIBK'
  ],
  install: true,
  install_dir: 'lib'
)
install_subdir('include', install_dir: 'include', strip_directory: true)

libk_dep = declare_dependency(link_with : libk, include_directories : includes)
