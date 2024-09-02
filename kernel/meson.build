srcs = [
  'asm.c',
  'cpu/pic.c',
  'cpu/interrupt.asm',
  'cpu/interrupt.c',
  'dev/serial.c',
  'dev/log.c',
  'globals.c',
  'kernel.c',
  'panic.c',
  'mem/mem.asm',
  'mem/mem.c',
  'mem/alloc.c',
  'mem/paging.c',
  'mem/paging.asm',
  'mem/pmm.c',
  'fs/fs.c',
  'fs/path.c'
]
link_script = meson.current_source_dir() / 'link.ld'

executable(
  'wyrm.bin',
  sources: srcs,
  include_directories: kernel_includes,
  nasm_args: [
    '-f',
    'elf64',
    '-Wall'
  ],
  c_args: c_args_freestanding,
  link_args: [
    '-T', link_script, 
    '-nostdlib' 
  ],
  link_depends: link_script,
  dependencies: [libk_dep, ktypes_dep],
  install: true,
  install_dir: 'boot'
)
install_subdir('include', install_dir: 'include/kernel', strip_directory: true)
install_data('limine.cfg', install_dir: 'boot/limine')
