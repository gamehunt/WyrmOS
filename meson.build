project(
  'WyrmOS', 
  ['c', 'nasm'],
  default_options: ['default_library=static'] 
)

c_args_freestanding = [
    '-Wall',
    '-Wextra',
    '-Wno-unused-function',
    '-Wno-unused-parameter',
    '-std=gnu11', 
    '-ffreestanding',
    '-O2',
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
    '-pipe',
    '-static'
]

c_args_user = [
    '-Wall',
    '-Wextra',
    '-Wno-unused-function',
    '-Wno-unused-parameter',
    '-std=gnu11', 
    '-O2',
    '-fno-stack-protector',
    '-fno-stack-check', 
    '-fno-lto',
    '-m64', 
    '-mno-mmx', 
    '-mno-sse', 
    '-mno-sse2', 
    '-mno-red-zone',
    '-g',
    '-pipe',
    '-static'
]

kernel_includes = include_directories('kernel/include')

subdir('lib')
subdir('kernel')
subdir('modules')
subdir('user')

meson.add_install_script('initrd.sh')
meson.add_install_script('sync-image.sh')

