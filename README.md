Vireo, a green thread library
=============================

Vireo is a tiny C library implementing preemptible green threads
(greenlets) for Linux. It is designed as an example of how to implement
userspace threading, that is, without special kernel support or
awareness of multiple threads. It is also a good example of how to use
the setcontext(3) family of functions.

Vireo's focus is on readability and pedagogy, not performance or
complete correctness. In particular, the library does no error-checking
for most system calls.

This library was written for MIT's operating systems class, and the
examples are taken from the examples / test cases in JOS, the teaching
OS used in that class. The library itself is licensed under the 2-clause
BSD license.
