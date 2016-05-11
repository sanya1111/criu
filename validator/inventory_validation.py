import os
from resgraph import *

def validate_inventory(inventory, errors_list):
    #check cgset
    errors_list.add_context("inventory")
    if not inventory.root_cg_set is None :
        if inventory.root_cg_set == 0:
            errors_list.add_error("wrong root cg set")
    errors_list.pop_context()
    
if __name__ == '__main__':
    dir_path = sys.argv[1]
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    inventory = Inventory(reader, errors)
    validate_inventory(inventory, errors)
    errors.show_errors()