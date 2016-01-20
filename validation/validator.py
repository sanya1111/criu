#!/usr/bin/env python

import sys
from validator import *


def main():
    dir_path = sys.argv[1]

    errors = ErrorList()
    reader = CriuImgFileReader(dir_path)
    processes = PsList(reader, errors)
    pstree = PsTree(processes, errors)

    validate(processes, pstree, errors, dir_path)

    processes.show_pslist()
    #pstree.show_pstree()
    errors.show_errors()


if __name__ == '__main__':
    main()
