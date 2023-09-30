# Go lnx parser

This directory contains an lnx file parser in Go.  This example
contains two files:
 - `pkg/lnxconfig/lnxconfig.go`:  The parser.  You should copy this
   into your repository using a similar directory structure to what
   you see here
- `cmd/example/main.go`:  Demo of how to import and use the parser.
  This example loads any lnx file and prints out the IPs assigned to
  this node.  
  

To build the example, run `make`, then run the compiled binary
`example` on any lnx file, as follows:

```
./example some-lnx-file.lnx
if0 has IP 10.1.0.2/24
if1 has IP 10.2.0.1/24
```
