#WyrmOS

Concept: 

From: https://en.wikipedia.org/wiki/Exokernel

Operating systems generally present hardware resources to applications through high-level abstractions such as (virtual) file systems. The idea behind exokernels is to force as few abstractions as possible on application developers, enabling them to make as many decisions as possible about hardware abstractions. Exokernels are tiny, since functionality is limited to ensuring protection and multiplexing of resources, which is considerably simpler than conventional microkernels' implementation of message passing and monolithic kernels' implementation of high-level abstractions.

Implemented abstractions are called library operating systems; they may request specific memory addresses, disk blocks, etc. The kernel only ensures that the requested resource is free, and the application is allowed to access it. This low-level hardware access allows the programmer to implement custom abstractions, and omit unnecessary ones, most commonly to improve a program's performance. It also allows programmers to choose what level of abstraction they want, high, or low.

WyrmOS should implement exokernel principles, providing in it's own kernel only resource managment, such as CPU, physical memory, storage, network e.t.c
All common abstraction will be implemented in WyrmLib, which will simplify development of native applications.

LibC headers will be independent on the underlaying LibOS, but binary can require some additional compatibility layer, since there is no unified syscalls.

#Install required tools

```
cd tools
./setup_env.sh
```

#Build

```
source tools/env.sh
./rebuild_and_launch.sh
```

#Launch without rebuild

`./launch.sh`
