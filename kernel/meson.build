srcs = [
  'asm.c',
  'cpu/interrupt.c',
  'dev/serial.c',
  'globals.c',
  'kernel.c',
  'mem/mem.c',
  'mem/paging.c',
  'mem/pmm.c',
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
    '-nostdlib', 
    '-pie',
  ],
  link_depends: link_script,
  dependencies: [libk_dep],
  install: true,
  install_dir: 'boot'
)
install_subdir('include', install_dir: 'include/kernel', strip_directory: true)
install_data('limine.cfg', install_dir: 'boot/limine')
