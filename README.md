# kdbOS
A personal OS as a hobby to learn how an OS works under the hood.

## Recommended Development Operating System
- Linux

## Necessary Installs
- nasm (the assembler)
- mkisofs (really this is genisoimage)

## How to run
To run this code on real hardware (x86_64 architectures), first make sure you have installed all the tools above! Then make sure the shell script has executable permissions on your machine, then simply run the shell script!

    > chmod +x build.sh
    > ./build.sh

This will generate an ISO file. Simply move the ISO file to a USB stick, then set your machine to boot from the USB, and voila!

**IMPORTANT**
You must enable Legacy Booting in your BIOS. This will not work with UEFI booting.

The program as of now simply prints Hello World! to the screen, then accepts keyboard input and echoes what the user types.

## Note
I tested this on a real laptop with an Intel Core i5 8th Gen processor. You could use a virtual machine, but I think running on real hardware is way cooler. If this doesn't work for you, honestly there could be a number of things that could have gone wrong.