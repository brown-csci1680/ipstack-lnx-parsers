# C++ lnx parser

This directory contains an lnx file parser in C++.  

To use the parser in your program, simply copy `lnxconfig.h` into
your project files and include it where you need it. There is no
need to modify your Makefile. See `demo.cpp` and the associated
Makefile for example usage.

## Example program
To build the example, run `make`, then run the binary `demo` on any
lnx file, as follows:

```
$ ./demo example-router.lnx
interface if0 10.0.0.2/24 127.0.0.1:5001
interface if1 10.1.0.1/24 127.0.0.1:5002
interface if2 10.3.0.1/24 127.0.0.1:5006
neighbor 10.0.0.1 at 127.0.0.1:5000 via if0
neighbor 10.1.0.2 at 127.0.0.1:5003 via if1
neighbor 10.3.0.2 at 127.0.0.1:5007 via if2
routing rip
rip advertise-to 10.1.0.2
rip advertise-to 10.3.0.2
```

For examples on how to use the structs and datatypes in the config file, see
`demo.cpp`, which contains helper methods for using each field.
