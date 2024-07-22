project(
  'WyrmOS', 
  ['c', 'nasm'],
  default_options: ['default_library=static'] 
)

c_args_freestanding = [
    '-Wall',
    '-Wextra',
    '-std=gnu11', 
    '-ffreestanding',
    '-fno-stack-protector',
    '-fno-stack-check', 
    '-fno-lto',
    '-m64', 
    '-mcmodel=large',
    '-march=x86-64', 
    '-mno-80387', 
    '-mno-mmx', 
    '-mno-sse', 
    '-mno-sse2', 
    '-mno-red-zone',
    '-g',
    '-O2',
    '-pipe'
]

kernel_includes = include_directories('kernel/include')

subdir('lib')
subdir('kernel')

meson.add_install_script('sync-image.sh')

