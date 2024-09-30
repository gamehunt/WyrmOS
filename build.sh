#!/bin/bash
set -e

meson compile crt0 k c -C build
yes | meson install --no-rebuild --tags crts -C build
yes | meson install --no-rebuild --tags libc -C build
meson compile -C build
yes | meson install -C build

