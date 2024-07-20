#!/bin/sh

rm -rf image.hdd

dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd
sgdisk image.hdd -n 1:2048 -t 1:ef00

${MESON_SOURCE_ROOT}/tools/limine/limine bios-install image.hdd

# Format the image as fat32.
mformat -i image.hdd@@1M

# Make relevant subdirectories.
mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT

# Copy over the relevant files.
mcopy -s -i image.hdd@@1M ${MESON_SOURCE_ROOT}/run/boot ::/
mcopy -i image.hdd@@1M ${MESON_SOURCE_ROOT}/tools/limine/limine-bios.sys ::/boot/limine
mcopy -i image.hdd@@1M ${MESON_SOURCE_ROOT}/tools/limine/BOOTX64.EFI ::/EFI/BOOT
mcopy -i image.hdd@@1M ${MESON_SOURCE_ROOT}/tools/limine/BOOTIA32.EFI ::/EFI/BOOT
