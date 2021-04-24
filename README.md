auxdump: Dump the ELF aux vector.
=================================

This tool dumps the ELF aux vector.  This vector is populated at ELF load by
the kernel.  Items in this vector include the kernel's page size and a random
value that can be used as a stack canary.

There are other similar utilities, sys/auxv.h manpage references the
environment variable LD_SHOW_AUXV=1 and sys/auxv.h declares getauxval().

Building
--------
1. Invoke `make` to build.

References
----------
* [glibc](https://www.gnu.org/software/libc/) source (rtld.c, dl-sysdep.c, libc-start.c).
* [linux kernel](https://www.kernel.org) source (binfmt_elf.c)
* sys/auxv.h: Declares getauxval().

Contact
-------
Matt Davis: https://github.com/enferex
