#!/bin/bash

TARGET=x86_64-elf
BINUTILS=2.42
GCC=14.1.0
PREFIX=$(realpath "./cross/prefix")
PATH=${PREFIX}/bin:${PATH}

function setup_binutils {
	if hash $TARGET-as >/dev/null 2>&1  
	then
		echo "Skipping binutils, already built"
		return
	fi

	mkdir -p binutils/build
	cd binutils
	
	if [ ! -f binutils-${BINUTILS}.tar.xz ]; then
		wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS}.tar.xz
		tar xf binutils-${BINUTILS}.tar.xz
	fi

	cd build
	../binutils-${BINUTILS}/configure --target=$TARGET --prefix=${PREFIX} --with-sysroot --disable-nls --disable-werror
	make
	make install
	cd ../..
}

function setup_gcc {
	if hash $TARGET-gcc >/dev/null 2>&1 
	then
		echo "Skipping gcc, already built"
		return
	fi

	mkdir -p gcc/build
	cd gcc
	
	if [ ! -f gcc-${GCC}.tar.xz ]; then
		wget https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-${GCC}.tar.xz
		tar xf gcc-${GCC}.tar.xz
	fi

	#compile libgcc without red zone
	if [ ! -f gcc-${GCC}/gcc/config/i386/t-x86_64-elf ]; then
		cp ../../t-x86_64-elf gcc-${GCC}/gcc/config/i386/
		sed -i '/x86_64-\*-elf\*)/a\'$'\n\t''tmake_file="${tmake_file} i386/t-x86_64-elf"' gcc-${GCC}/gcc/config.gcc
	fi

	cd build
	../gcc-${GCC}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
	make -j 8 all-gcc
	make all-target-libgcc
	make install-gcc
	make install-target-libgcc
	cd ../..
}

function setup_limine {
	if [ -f limine/limine ]; then
		echo 'Skipping limine, already exists...'
		return
	fi
	git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1
	make -C limine	
}

function setup_hdd {
	if [ -f image.hdd ]; then
		echo 'Skipping image, already exists...'
		return
	fi

	dd if=/dev/zero count=0 seek=4194304 of=image.hdd
	sgdisk image.hdd -n 1:2048:128M -t 1:ef00
	sgdisk image.hdd -N 2           -t 2:8300

	sudo -u root kpartx -a image.hdd
	
	sudo -u root mkfs.fat -F 32 /dev/mapper/loop0p1
	sudo -u root mkfs.ext4 /dev/mapper/loop0p2

	mkdir -p /tmp/wyrmos_root
	sudo -u root mount /dev/mapper/loop0p1 /tmp/wyrmos_root 

	./limine/limine bios-install image.hdd

	sudo -u root mkdir -p /tmp/wyrmos_root/EFI/BOOT
	sudo -u root mkdir -p /tmp/wyrmos_root/boot/limine

	sudo -u root cp limine/limine-bios.sys /tmp/wyrmos_root/boot/limine/
	sudo -u root cp limine/BOOTX64.EFI /tmp/wyrmos_root/EFI/BOOT/
	sudo -u root cp limine/BOOTIA32.EFI /tmp/wyrmos_root/EFI/BOOT/

	sudo -u root umount /tmp/wyrmos_root
	sudo -u root kpartx -d image.hdd

	sudo -u root rm -rf /tmp/wyrmos_root

	sync
}

mkdir -p cross/prefix
cd cross

setup_binutils
setup_gcc

cd ..

setup_limine
setup_hdd


