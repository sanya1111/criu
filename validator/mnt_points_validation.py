from resgraph.functional_divider import extra_check, enable_extra
from resgraph.elements import MntPointList, MntPointDict, PsList, PsTree, Inventory
from resgraph import ErrorList, CriuDumpReader
import sys
import pycriu
import os
import re
from collections import defaultdict
from common_checks import *

'''
LEFT
ext_mounts
internal sharing flag WTF
deleted
'''
    
def check_fstype(mnt, errors_list):
    if not mnt.fstype in pycriu.images.mnt_pb2.fstype.values():
        errors_list.add_error("wrong type")
        
def do_check_mnt_id(id, errors):
    try: 
        int(id)
    except ValueError:
        errors.add_error("wrong mnt_id")
    '''
    here can be comparing id with global mounts
    '''
    pass

def check_mnt_id(mnt, errors):
    do_check_mnt_id(mnt.mnt_id, errors)


def check_root_dev(mnt, errors):
    ''' 
    reader wrong parses root_dev field - it parses it to int
    here should be
    if not re.match("^\d+:\d+$", str(mnt.root_dev)):
        errors.add_error("wrong root dev")
    '''
    pass
        
def check_parent_mnt_id(mnt, errors):
    do_check_mnt_id(mnt.parent_mnt_id, errors)
        
def check_flags(mnt, errors):
    '''
    currently allowable flags are
    MS_SHARED | MS_PRIVATE |
    MS_SLAVE | MS_UNBINDABLE |
    MS_NOSUID | MS_NODEV | MS_NOEXEC |
    MS_NOATIME | MS_NODIRATIME | MS_RELATIME
    
    problem is that they places in fs.h module, which is not platform independent
    '''
    pass

def check_root(mnt, errors_list):
    if not check_path(mnt.root):
        errors_list.add_error("wrong root")

def check_mountpoint(mnt, errors_list):
    if not check_path(mnt.mountpoint):
        errors_list.add_error("wrong root")
        
def check_options(mnt, errors_list):
    '''
    it's strong question, what to do there.
    variants
    1) subprocess.popen("mount --options") or some else mount util ( we trying to mount and parse error if exists)
    2) copy options parser from mount command source 
    '''
    pass

def check_shared_inheritance(mnt_list, errors):
    errors.add_context("check shared_inheritance")
    edges = defaultdict(list)
    for item in mnt_list:
        if item.master_id != None and item.shared_id == None:
            errors.add_error("master id exists but shared id don't")
            errors.pop_context()
            return
        if item.shared_id != None and item.master_id != None:
            edges[item.master_id].append(item.shared_id)
            

    used = set()
    do_check_inheritance(0, edges, used, errors)
    errors.pop_context()
        
def check_mntid_inheritance_add_edge(parent, child, edges, errors_list):
    if not child.mountpoint.startswith(parent.mountpoint):
        errors_list.add_error("{0} is parent of {1}, but there mntpoints isn't inserted")
    else:
        edges[parent].append(child)
    
        
def check_mntid_inheritance(mnt_list, errors_list):
    errors_list.add_context("check inheritance")
    mnt_id_dict = {}
    for item in mnt_list:
        mnt_id_dict[item.mnt_id] = item
    tree_roots = []
    tree_edges = defaultdict(list)
    used = set()
    for item in mnt_list:
        parent = item.parent_mnt_id
        if parent != item.mnt_id:
            if not parent in mnt_id_dict:
                tree_roots.append(item)
            else:
                check_mntid_inheritance_add_edge(mnt_id_dict[parent], item, tree_edges, errors_list)
    for root in tree_roots:
        do_check_inheritance(root, tree_edges, used, errors_list)
    errors_list.pop_context()
    
    
@extra_check
def validate_one_mnt(mnt, errors):
    errors.add_context("validate mnt with id {}".format(mnt.mnt_id))
    check_fstype(mnt, errors)
    check_mnt_id(mnt, errors)
    check_root_dev(mnt, errors)
    check_parent_mnt_id(mnt, errors)
    check_flags(mnt, errors)
    check_root(mnt, errors)
    check_mountpoint(mnt, errors)
    check_options(mnt, errors)
    errors.pop_context()
    
@extra_check
def validate_mnt_list(mntpoint_list, errors_list):
    errors_list.add_context("mnts validating")
    for item in mntpoint_list:
        validate_one_mnt(item, errors_list)
    check_mntid_inheritance(mntpoint_list, errors_list)
    check_shared_inheritance(mntpoint_list, errors_list)
    errors_list.pop_context()
    
@extra_check
def validate_mnt_dict(mntpoint_dict, errors_list):
    for list_item in mntpoint_dict:
        errors_list.add_context("validate mntlist for {}".format(list_item[0]))
        validate_mnt_list(list_item[1], errors_list)
        errors_list.pop_context()

if __name__ == '__main__':
    dir_path = sys.argv[1]
    errors = ErrorList()
    enable_extra()
    reader = CriuDumpReader(dir_path)
    processes = PsList(reader, errors)
    pstree = PsTree(processes, errors)
    inventory = Inventory(reader, errors)
    dd = MntPointDict(inventory, pstree, reader, errors)
    validate_mnt_dict(dd, errors)
#     li = MntPointList(8, reader, errors) 
#     validate_mnts(li, errors)
    errors.show_errors()