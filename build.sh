#!/bin/bash
set -e

meson compile -C build
sudo -u root meson install -C build

