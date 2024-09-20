#!/bin/bash

qemu-system-x86_64 \
	-bios /usr/share/OVMF/x64/OVMF_CODE.fd \
	-M q35 \
	-m 8G \
	-net none \
	-enable-kvm \
    -smp 4 \
	-drive file=tools/image.hdd,format=raw
