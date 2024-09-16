#!/bin/bash
set -e

meson compile crt0 -C build
meson install --no-rebuild --tags crts -C build
meson compile -C build
meson install -C build

