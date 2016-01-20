#!/usr/bin/env python

import sys
from resgraph import PsList, PsTree, ErrorList, CriuDumpReader
from validation import validate

def main():
    dir_path = sys.argv[1]

    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    processes = PsList(reader, errors)
    pstree = PsTree(processes, errors)

    validate(processes, pstree, errors, dir_path)

    processes.show_pslist()
    #pstree.show_pstree()
    errors.show_errors()


if __name__ == '__main__':
    main()
