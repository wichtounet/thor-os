Thor Operating System
=====================

.. image:: https://github.com/wichtounet/thor-os/wiki/images/thor-vesa.png

Thor is an operating system created for learning purposes and for fun.

It is currently a 64-bit OS, written mainly in C++, with a few lines of assembly when necessary.

Features
########

* 64-bit operating system (x86_64 architecture only)
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

Why?...
#######

There are many reasons to build an operating system from scratch.

For me, the main two reasons are for fun and to learn new things. When I grew bored of working on my custom compiler, I decided to work on a new project and I chose Operating System Development. It's probably the most complicated hobby project that can be done. I had to learn plenty of things to be advance during this project. There are tons of difficulty that you don't even think about in normal programs. More generally, since I love programing, I was sure I could have a lot of fun developping my own OS. There is also the geek side of having its own OS :)

I have absolutely no intentions on making money with this project. When I no longer have fun developing it, I'll simply switch project for a while ;)

... and How?
############

Writing an operating system from scratch is not easy and at each step you need a lot of information that is not necessarily easily available. Generally, the more advanced your OS is, the less information you'll find.

The main two resources for develpment have been:

* `The osdev.org Wiki <http://wiki.osdev.org/Main_Page>`_ : Tons of information of various subjects.
* `The osdev.org Forum <http://forum.osdev.org/index.php>`_ : Tons of great posts and a lot of very skilled people to help you if you have a specific issue.

A good series of tutorials to start with is the `Broken Thorn series <http://www.brokenthorn.com/Resources/OSDevIndex.html>`_. I haven't followed the complete tutorial (only some specific parts), but it's full of good information and it's probably a good place to start.

There are also the `James Molloy Series <https://web.archive.org/web/20160301082842/http://www.jamesmolloy.co.uk/tutorial_html/index.html>`_. It's much less complete, but if you like tutorials, it's probably going to be helpful.

And of course (if you develop on Intel), the *Intel 64 and IA-32 Architectures Software Developer's Manual*, there are several parts to the guide, I've mainly used the *System Programming - Part 1* part. Although it can seem heavy, it's pretty good and will be an invaluable asset to understand the architecture: for instance, paging and interrupts handling.

As for Books, I've been pretty much disappointed so far. There are plenty of books on operating system, but none of the few I've read so far have been great. They are probably good at helping you understand OSes, but pretty bad at helping you develop your own OS. Moreover, they are also pretty much outdated.

The books I've read are:

* Modern Operating Systems by Andrew S. Tanenbaum, 3rd ed. It's a pretty good book overall. It covers most of the subjects, although it doesn't go in too much details on some subjects (network implementation). Moreover, it also covers some subjects that most hobbyists are not going to care about (lots about security for instance).
* Operating Systems: Design and Implementation by Andrew S. Tanenbaum et al. I would only advise this book if you want to learn about MINIX, if you don't care about MINIX, don't read this book. It's full of code, not really well organized and not easy to follow.

In general, while books will help you understand operating systems a bit, they won't really help you develop anything. I've other books that should arrive soon, I hope they'll turn out better.

Build
#####

Build procedure is described on the Wiki: https://github.com/wichtounet/thor-os/wiki/Build

License
#######

This project is distributed under the MIT License. Read `LICENSE` for details.
