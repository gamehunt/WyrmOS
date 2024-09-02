#!/bin/bash
set -e

cd ${MESON_SOURCE_ROOT}/run
tar -cf boot/wyrm.initrd initrd
