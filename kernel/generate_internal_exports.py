#!/bin/python

import fileinput
import sys

exclusions = [
    'current_core'
]

f = open(sys.argv[1], "w")
f.write("#include <symbols.h>\n\n")

for line in fileinput.input():
    words  = line.split(' ')
    sanitized_word = words[2].strip("\r\n ")
    if(words[1] != 'N' and 
       not sanitized_word.startswith('__export') and 
       not ("." in sanitized_word) and 
       not (sanitized_word in exclusions)):
        export = f"EXPORT_INTERNAL({sanitized_word}, 0x{words[0]})\n"
        f.write(export)

f.close()
