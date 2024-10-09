srcs = [
  'main.c',
]

executable('wsh', 
  sources: srcs,
  c_args: [
    c_args_user,
    '-O0'
  ],
  install: true,
  install_dir: 'initrd/bin',
  dependencies: [libc_dep]
)
