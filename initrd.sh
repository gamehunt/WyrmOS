#!/bin/bash
set -e

cd ${MESON_SOURCE_ROOT}/run/initrd
tar -cf ../boot/wyrm.initrd *
