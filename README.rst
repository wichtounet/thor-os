Thor Operating System
=====================

Thor is an operating system created for learning purposes and for fun.

It is currently a 64bit OS written mainly in C++, with few lines of assembly when necessary.

Features
########

* 64bit operating system (x86_64 architecture only)
* Preemptive Multiprocessing
* Applications written directly in C++ with a system library (loaded with ELF)
* Keyboard / Mouse driver
* Full ACPI support with ACPICA
* Read/Write ATA driver
* FAT32 file system support
* HPET/RTC/PIT drivers
* Basic networking support (WIP) with Realtek 8139 driver
* Basic PCI support
* Multi stage booting with FAT32

Build
#####

Build procedure is described on the Wiki: https://github.com/wichtounet/thor-os/wiki/Build

License
#######

This project is distributed under the Boost Software License 1.0. Read `LICENSE` for details.

(This is likely to change to the MIT License)
