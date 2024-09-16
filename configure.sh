#!/bin/bash

meson setup --cross-file tools/x86_64-wyrm.txt --prefix=$(realpath ./run) build
