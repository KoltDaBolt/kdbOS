# kdbOS
A personal OS as a hobby to learn how an OS works under the hood.

## Recommended Development Operating System
- Linux (I used Debian / Ubuntu / WSL)

## Necessary Installs
- binutils
- GCC (Cross Compiler)
    - i686-elf-gcc
    - i686-elf-ld
- fasm
- genisoimage
- QEMU (if not running on real hardware)

## How to Setup Binutils and GCC Cross Compiler
**NOTE**
You do not need to set these up in the same folder as this project. These can be wherever you like.

Step 1: Download and install source code

    > sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev textinfo
    > mkdir cross-compiler
    > cd cross-compiler
    > wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
    > wget https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz
    > tar -xvf binutils-2.40.tar.gz
    > tar -xvf gcc-12.3.0.tar.gz

Step 2: Build Binutils

    > mkdir build-binutils
    > cd build-binutils
    > ../binutils-2.40/configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --disable-werror
    > make
    > sudo make install
    > cd ..

Step 3: Build GCC

    > mkdir build-gcc
    > cd build-gcc
    > ../gcc-12.3.0/configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --enable-languages=c --without-headers
    > make all-gcc
    > sudo make install-gcc
    > cd ..

Step 4: Add to PATH

    > export PATH="/usr/local/cross/bin:$PATH"

## How to Run (Currently Only Works with Qemu)
To run this code on the QEMU emulator, run the following commands:

    > make init *Only run once after cloning*
    > make qemu
    > qemu-system-x86_64 -hda ./bin/os.bin
    > make clean *Clean up bin and build folders before building again*

To run this code on real hardware (x86_64 architectures), run the following commands:

    > make init *Only run once after cloning*
    > make
    > make clean *Clean up bin and build folders before building again*

This will generate an ISO file. Simply move the ISO file to a USB stick, then set your machine to boot from the USB.

**IMPORTANT**
You must enable Legacy Booting in your BIOS. This will not work with UEFI booting.
