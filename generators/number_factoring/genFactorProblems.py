#!/usr/bin/env python

__author__ = "Heidi Dixon"

import os
import sys

parameter1 = "n-bit" # n-bit or fast
parameter2 = "carry-save" # carry-save wallace recursive

def generateFactorProblems(bottom,top):
    if bottom <= top:            
        for i in range(bottom,top):
            filename = "nf." + str(i) + parameter1 + "." + parameter2 + ".cnf"
            os.system("nftocnf " + str(i) + " " + parameter1 + " " + parameter2 + " >  " + filename)


if __name__ == "__main__":
    generateFactorProblems(int(sys.argv[1]),int(sys.argv[2]))
