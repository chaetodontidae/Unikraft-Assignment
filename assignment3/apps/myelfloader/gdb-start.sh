#!/bin/bash
# Host and port of the GDB server port (qemu)
GDBSRV=":1234"

# Generate a GDB command line for loading the symbols of an ELF executable/
# shared library.
# Usage: gdb-add-symbols "<ELF executable/library>" \
#                        "<base load address (hex, no leading '0x')>"
gdb-add-symbols()
{
	local LOAD_ELF="$1"
	local LOAD_ADDR="${2}"
	local LOAD_TADDR=
	local TEXT_OFFSET=

	# Hacky way to figure out the .text offset
	TEXT_OFFSET=$( readelf -S "${LOAD_ELF}" | grep '.text' | awk '{ print $5 }' )

	# Compute offset of .text section with base address
	LOAD_TADDR=$( printf 'obase=16;ibase=16;%s+%s\n' "${LOAD_ADDR^^}" "${TEXT_OFFSET^^}" | bc )

	# Generate GDB command
	printf 'add-symbol-file -readnow %s 0x%s' "${LOAD_ELF}" "${LOAD_TADDR}"
}

# Connect to $GDBSRV and set up gdb
# NOTE: The first block of instructions connects to the gdb port of the qemu
#       process and follows the CPU mode change while the guest is booting.
#       This is currently a requirement for using gdb with qemu for x86_64. For
#       other platforms and architectures, this may be different.
#       This block assumes that the guest is started in paused state (`-P` for
#       `qemu-guest`).
# NOTE: Please note that regular breakpoints within the application or a shared
#       library can only be set after they have been loaded into memory. Usually
#       this is done by elfloader for the application and dynamic loader or by
#       the dynamic loader for shared libraries.
# HINT: You can use the `directory` command to specify additional paths that
#       `gdb` will use to search for source files.
#       For example, if you run your dynamically linked application with
#       Debian's libc, you can install (`apt install glibc-source`) and
#       extract the glibc sources under /usr/src/glibc.
#         --eval-command="directory /usr/src/glibc/glibc-2.31"
exec gdb \
	--eval-command="target remote $GDBSRV" \
	--eval-command="hbreak _ukplat_entry" \
	--eval-command="continue" \
	--eval-command="disconnect" \
	--eval-command="set arch i386:x86-64:intel" \
	--eval-command="target remote $GDBSRV" \
	\
	--eval-command="$( gdb-add-symbols "rootfs/helloworld" "8000000000" )" \
	--eval-command="$( gdb-add-symbols "rootfs/libc.so.6" "8000005000" )"
