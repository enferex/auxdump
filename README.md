auxdump: Dump the ELF aux vector.
=================================

This tool dumps the ELF aux vector.  This vector is populated at ELF load by
the kernel.  Items in this vector include the kernel's page size and a random
value that can be used as a stack canary.

Building
--------
1. Invoke `make` to build.

References
----------
* [glibc](https://www.gnu.org/software/libc/) source (rtld.c, dl-sysdep.c, libc-start.c).
* [linux kernel](https://www.kernel.org) source (binfmt_elf.c)

Contact
-------
Matt Davis: https://github.com/enferex
