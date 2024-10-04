srcs = [
  'main.c',
]

ata_mod = executable('ata', 
  sources: srcs,
  include_directories: [includes, kernel_includes],
  c_args: [
    c_args_freestanding,
    '-D__MODULE'
  ],
  link_args: [
    '-r',
    '-nostdlib',
  ],
  dependencies: [libc_h_dep],
  install: true,
  install_dir: 'initrd/modules',
  name_prefix: '',
  name_suffix: 'wrm'
)
