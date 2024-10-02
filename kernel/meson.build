srcs = [
  'asm.c',
  'symbols.c',
  'cpu/syscall.c',
  'dev/serial.c',
  'dev/log.c',
  'dev/pci.c',
  'dev/acpi.c',
  'dev/null.c',
  'dev/random.c',
  'dev/terminal.c',
  'globals.c',
  'kernel.c',
  'panic.c',
  'mem/alloc.c',
  'mem/pmm.c',
  'mem/mmio.c',
  'fs/fs.c',
  'fs/path.c',
  'exec/initrd.c',
  'exec/elf.c',
  'exec/module.c',
  'exec/exec.c',
  'proc/process.c',
]
link_script = meson.current_source_dir() / arch_folder / 'link.ld'

subdir(arch_folder)

foreach src : arch_srcs
    srcs += '/'.join([arch_folder, src])
endforeach


objcopy = find_program('objcopy')
font = custom_target(
  'font.o',
  output: 'font.o',
  input: 'font.psf',
  command: [objcopy, '-O', 'elf64-x86-64', '-B', 'i386', '-I', 'binary' ,'@INPUT@', '@OUTPUT@']
)

prebuilt_kernel = executable(
  'wyrm.bin.0',
  sources: [srcs, font],
  include_directories: [kernel_includes, arch_includes],
  nasm_args: [
    '-f',
    'elf64',
    '-Wall'
  ],
  c_args: [
    c_args_freestanding,
    '-D__KERNEL',
    '-DNO_LOCK_CHECKS',
    arch_define,
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
    '-D__KERNEL',
    '-DNO_LOCK_CHECKS',
    arch_define,
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
