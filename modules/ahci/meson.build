srcs = [
  'main.c',
]

ahci_mod = executable('ahci', 
  sources: srcs,
  include_directories: [includes, kernel_includes, kernel_arch_includes],
  c_args: [
    c_args_freestanding,
    '-D__MODULE',
    arch_define
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
