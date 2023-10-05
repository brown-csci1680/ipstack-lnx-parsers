# C lnx parser

This directory contains an lnx file parser in C.  

The most important files are:
 - `lnxconfig.{c,h}`:  These files define the parser
 - `demo.c` Example on how to use the parser and traverse the config
   struct
   
To use the parser in your program, copy `lnxconfig.c` and
`lnxconfig.h` into your program wherever you store library code.  See
the `Makefile` for examples on how to build your code from multiple
object files.  

**IMPORTANT NOTE**: This parser uses the `list.h` linked-list
implementation provided in the
[c-utils](https://github.com/brown-csci1680/c-utils) repo.  If you are
using different list.h, you may need to rename the list.h here to
avoid conflicts.
   
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
`demo.c`, which contains helper methods for using each field.
