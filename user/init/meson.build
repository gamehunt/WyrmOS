srcs = [
  'main.c',
]

executable('init', 
  sources: srcs,
  c_args: [
    c_args_user,
    '-O0'
  ],
  install: true,
  install_dir: 'initrd/bin',
  dependencies: [libc_dep]
)
