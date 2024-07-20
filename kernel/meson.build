includes = include_directories('include')

srcs = [
  'src/kernel.c',
]
link_script = meson.current_source_dir() / 'link.ld'

executable(
  'wyrm.bin',
  sources: srcs,
  include_directories: includes,
  nasm_args: [
    '-f',
    'elf64',
    '-Wall'
  ],
  c_args: [
    '-Wall',
    '-Wextra',
    '-std=gnu11', 
    '-ffreestanding',
    '-fno-stack-protector',
    '-fno-stack-check', 
    '-fno-lto',
    '-fPIE',
    '-m64', 
    '-march=x86-64', 
    '-mno-80387', 
    '-mno-mmx', 
    '-mno-sse', 
    '-mno-sse2', 
    '-mno-red-zone',
    '-g',
    '-O2',
    '-pipe'
  ],
  link_args: [
    '-T', link_script, 
    '-nostdlib', 
    '-pie',
  ],
  link_depends: link_script,
  install: true,
  install_dir: 'boot'
)
install_subdir('include', install_dir: 'include/kernel', strip_directory: true)
install_data('limine.cfg', install_dir: 'boot/limine')
