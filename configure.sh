#!/bin/bash

meson setup --cross-file tools/x86_64-elf.txt --prefix=$(realpath ./run) build
