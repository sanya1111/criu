import os
import re
def check_is_int(field):
    try:
        int(field)
        return True
    except:
        return False
    

def check_is_negative(field):
    return field < 0
    
def check_path(path):
    return os.path.isabs(path) and re.match("[^\0]+", path)

def check_open_flags(flags, other_flags=[]):
    other_flags = other_flags + [os.O_RDONLY, os.O_WRONLY, os.O_RDWR]
    for flag in other_flags:
        if (flag | flags) == flags:
            flags ^= flag
    return flags == 0

def do_check_unique_for_field(data, field_name, errors):
    dict = set()
    for item in data:
        field = item.__dict__[field_name]
        if field in dict:
            errors.add_error("several equals ids with value = {}".format(field))
        else:
            dict.add(field)
    
def do_check_inheritance(item, edges, used, errors_list):
    used.add(item)
    for child in edges[item]:
        if child in used:
            errors_list.add_error("cycle found")
            return
        do_check_inheritance(child, edges, used, errors_list)