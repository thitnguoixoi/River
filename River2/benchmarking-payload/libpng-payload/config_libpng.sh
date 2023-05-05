#! /bin/bash

# Init source code
SRC_DIR="$(pwd)/libpng-src"

DEFAULT_CFLAGS="-m32 -fno-stack-protector"
DEFAULT_LDFLAGS="-m32"
CC=gcc
CXX=g++

init_target() {
	if ! [ -d $SRC_DIR ]; then
		wget https://github.com/glennrp/libpng/archive/v1.6.33.tar.gz
		tar xvf v1.6.33.tar.gz
		mv libpng-1.6.33 $SRC_DIR
		cd $SRC_DIR
		autoreconf -fiv
		cd -
	fi
}


# Build http-parser with the given `name` and flags.
build_target() {
	cd $SRC_DIR
	make clean
	CC="$CC" CFLAGS="$DEFAULT_CFLAGS" LDFLAGS="$DEFAULT_LDFLAGS" \
	LD="ld -m elf_i386"  ./configure --enable-werror=no \
	--enable-hardware-optimizations=no --enable-shared=no \
	--enable-unversioned-links=no

	make libpng16.la
	cd -
}

init_target
build_target
