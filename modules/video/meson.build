srcs = [
  'main.c',
]

video_mod = executable('video', 
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
  dependencies: [libk_dep],
  install: true,
  install_dir: 'initrd/modules',
  name_prefix: '',
  name_suffix: 'wrm'
)
