import sys

from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList, PsTree
from resgraph.elements import RegFilesList, FdInfoDict, MntPointDict
from resgraph.functional_divider import *
import common_checks

'''
LEFT

fown

'''
def check_pipe_id(regfile, errors):
    if not common_checks.check_is_int(regfile.pos):
        errors.add_error("wrong id")

def check_flags(regfile, errors):
    '''
    WTF takes places in regfile's flags - at one case it is empty, at other - str like O_LARGEFILE | 0x80000 ?
    '''
    pass


def check_pos(regfile, errors):
    if not common_checks.check_is_int(regfile.pos):
        errors.add_error("wrong pos")   
    '''
    smth else ? - fstat we cant use - depends on abs path, which depends on mount'
    '''
    pass

def check_fown(regfile, errors):
    pass

def check_name(regfile, errors):
    if not common_checks.check_path(regfile.name):
        errors.add_error("wrong name")
        
def check_mnt_id(regfile, errors):
    if regfile.mnt_id != None and not common_checks.check_is_int(regfile.mnt_id):
        errors.add_error("wrong mnt_id")

def check_compatibility_with_mnts(regfile, mntpoint_dict, errors):
    if regfile.mnt_id != None:
        for list_item in mntpoint_dict:
            for item in list_item[1]:
                if item.mnt_id == regfile.mnt_id and not regfile.name.startswith(item.mountpoint):
                    errors.add_error("mnt id {} specified, but name not startswith mountpoint".format(regfile.mnt_id))
    
    
@extra_check
def validate_one_regfile(regfile, errors, mntpoint_dict):
    errors.add_context("validating regfile {}".format(regfile.id))
    check_flags(regfile, errors)
    check_pos(regfile, errors)
    check_fown(regfile, errors)
    check_name(regfile, errors)
    check_name(regfile, errors)
    check_mnt_id(regfile, errors)
    if mntpoint_dict != None:
        check_compatibility_with_mnts(regfile, mntpoint_dict, errors)
    errors.pop_context()
    
@extra_check
def validate_reg_files_list(reg_files_list, mntpoints_dict, errors):
    errors.add_context('validating regfiles list')
    for regfile in reg_files_list:
        validate_one_regfile(regfile, errors, mntpoints_dict)
    errors.pop_context()

if __name__ == "__main__":
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    processes = PsList(reader, errors) 
    pstree = PsTree(processes, errors)
    inventory = Inventory(reader, errors)
#     fuck = FdInfoDict(inventory, processes, reader, errors)
#     for list_item in fuck:
#         for item in list_item[1]:
#             print(item)
#             print
#             
#     print
#     validate_files_ids(inventory, reader, processes, errors)
    
    reg_files_list = RegFilesList(reader, errors)
    mnts = MntPointDict(inventory, pstree, reader, errors)
    validate_reg_files_list(reg_files_list, mnts, errors)
    
    for item in reg_files_list:
        print(item)
        print
    errors.show_errors()