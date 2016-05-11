from collections import defaultdict
import os
import sys

from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList
from resgraph.elements import FdInfoDict
from resgraph.functional_divider import enable_extra
import common_checks

def check_file_id(ids, errors):
    common_checks.check_is_int(ids.files_id)
    '''
    combility checks inside FdInfoDict.__init__
    '''
    pass      

def check_ids(ids, errors):
    check_file_id(ids, errors)

def validate_kobj_ids(inventory, processes, errors_list):
    for proc in processes:
        for thread in proc.threads:
            check_ids(thread.ids, errors_list)
    if inventory.root_ids != None:
        check_ids(inventory.root_ids, errors_list)
        
    
if __name__ == '__main__':
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    processes = PsList(reader, errors) 
    inventory = Inventory(reader, errors)
    pipes_data_list = FdInfoDict(inventory, processes, reader, errors)
    errors.show_errors()