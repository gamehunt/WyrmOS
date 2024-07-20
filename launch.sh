#!/bin/bash

qemu-system-x86_64 \
	-bios /usr/share/OVMF/x64/OVMF_CODE.fd \
	-M q35 \
	-m 8G \
	-net none \
	-enable-kvm \
	-drive file=build/image.hdd,format=raw
