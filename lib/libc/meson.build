includes = include_directories('include')

srcs = [
  'assert.c',
  'ctype/ctype.c',
  'string/memory.c',
  'string/string.c',
  'stdio/printf.c',
  'stdlib/stdlib.c',
  'stdlib/alloc.c'
]

user_srcs = [
  'unistd.c',
  'stdio/stdio.c',
  'stdlib/_alloc.c',
  'wyrm/syscall.asm',
  'sys/mman.c',
  'signal.c',
  'libc.c',
]

libk = static_library('k', 
  sources: srcs,
  include_directories: [includes, kernel_includes],
  c_args: [
    c_args_freestanding,
    '-D__LIBK'
  ],
  install: true,
  install_dir: 'lib',
  install_tag: 'libc',
)

nasm = find_program('nasm')
crt0 = custom_target('crt0',
  output : 'crt0.o',
  input : 'crt0.asm',
  command : [nasm, '@INPUT@',  '-felf64', '-o', '@OUTPUT@'],
  install : true,
  install_tag: 'crts',
  install_dir : 'lib'
)

libc = static_library('c', 
  sources: [srcs, user_srcs],
  include_directories: [includes, kernel_includes],
  c_args: [
    c_args_freestanding,
    '-D__LIBC'
  ],
  install: true,
  install_tag: 'libc',
  install_dir: 'lib'
)

install_subdir('include', install_dir: 'include', strip_directory: true)

libk_dep   = declare_dependency(link_with : libk, include_directories : includes)

libc_h_dep = declare_dependency(include_directories : includes)
libc_dep = declare_dependency(link_with : libc, include_directories : includes)
