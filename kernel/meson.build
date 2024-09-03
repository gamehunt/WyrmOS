srcs = [
  'asm.c',
  'symbols.c',
  'cpu/pic.c',
  'cpu/interrupt.asm',
  'cpu/interrupt.c',
  'dev/serial.c',
  'dev/log.c',
  'dev/pit.c',
  'dev/timer.c',
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
  'fs/path.c',
  'exec/initrd.c',
  'exec/elf.c',
  'exec/module.c',
  'exec/exec.c',
  'proc/process.c',
  'proc/process.asm'
]
link_script = meson.current_source_dir() / 'link.ld'

prebuilt_kernel = executable(
  'wyrm.bin.0',
  sources: srcs,
  include_directories: kernel_includes,
  nasm_args: [
    '-f',
    'elf64',
    '-Wall'
  ],
  c_args: [
    c_args_freestanding,
    '-D__KERNEL'
  ],
  link_args: [
    '-T', link_script, 
    '-nostdlib',
  ],
  link_depends: link_script,
  dependencies: [libk_dep, ktypes_dep],
)

internal_exports = custom_target(
  'internal_exports.c', 
  output: 'internal_exports.c',
  input: [prebuilt_kernel, 'generate_internal_exports.py'],
  command: ['./exp_gen_wrapper.sh', '@INPUT@', '@OUTPUT@', '@INPUT1@'],
  depends: [prebuilt_kernel]
)

executable(
  'wyrm.bin',
  sources: internal_exports,
  include_directories: kernel_includes,
  c_args: [
    c_args_freestanding,
    '-D__KERNEL'
  ], 
  link_args: [
    '-T', link_script, 
    '-nostdlib',
  ],
  link_depends: link_script,
  objects: prebuilt_kernel.extract_all_objects(),
  dependencies: [libk_dep, ktypes_dep],
  install: true,
  install_dir: 'boot'
)

install_subdir('include', install_dir: 'include/kernel', strip_directory: true)
install_data('limine.cfg', install_dir: 'boot/limine')
