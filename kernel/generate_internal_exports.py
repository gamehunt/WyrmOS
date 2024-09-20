#!/bin/python

import fileinput
import sys

f = open(sys.argv[1], "w")
f.write("#include <symbols.h>\n\n")

for line in fileinput.input():
    words  = line.split(' ')
    if(words[1] != 'N' and not words[2].startswith('__export') and not "." in words[2]):
        export = f"EXPORT_INTERNAL({words[2].strip("\r\n ")}, 0x{words[0]})\n"
        f.write(export)

f.close()
