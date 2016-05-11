import os
from resgraph import *
import common_checks
import stat

def check_pipe_id(fdinfo, errors):
    if not common_checks.check_is_int(fdinfo.id):
        errors.add_error("wrong id")
        
def check_flags(fdinfo, errors):
    if not common_checks.check_open_flags(fdinfo.flags):
        errors.add_error("wrong flags") 
    pass
    
def check_type(fdinfo, errors_list):
    if not fdinfo.type in pycriu.images.fdinfo_pb2.__dict__:
        errors_list.add_error("wrong type")
        
def check_fd(fdinfo, errors):
    if not common_checks.check_is_int(fdinfo.id):
        errors.add_error("wrong fd")
        
def check_fd_used_once(fdinfo_list, errors):
    common_checks.do_check_unique_for_field(fdinfo_list, "fd", errors)
        
def check_compability_with_resources_with_id(fdinfo, resources, errors):
    type = fdinfo.type
    if type == 'REG' and resources.regfiles != None:
        if filter(lambda item : item.id == fdinfo.id, resources.regfiles) == []:
            errors.add_error("wrong id with regfiles compablity")
            
    if type == 'PIPE' and resources.pipes != None:
        if filter(lambda item : item.id == fdinfo.id, resources.pipes) == []:
            errors.add_error("wrong id with pipes compablity")
            
    if type == 'FIFO' and resources.fifos != None:
        if filter(lambda item : item.id == fdinfo.id, resources.fifos) == []:
            errors.add_error("wrong id with fifos compablity")
    
    if type == 'INETSK' and resources.intesk != None:
        if filter(lambda item : item.id == fdinfo.id, resources.intesk) == []:
            errors.add_error("wrong id with intesk compablity")
        
@extra_check
def validate_one_fdinfo(fdinfo, resources, errors):
    errors.add_context("validating fdinfo {}".format(fdinfo))
    check_pipe_id(fdinfo, errors)
    check_flags(fdinfo, errors)
    check_type(fdinfo, errors)
    check_fd(fdinfo, errors)
    check_compability_with_resources_with_id(fdinfo, resources, errors)
    errors.pop_context()
            
@extra_check
def validate_fdinfo_list(fdinfo_list, resources, errors):
    errors.add_context("validating fdinfo_list")
    for fdinfo in fdinfo_list:
        validate_one_fdinfo(fdinfo, resources, errors)
    check_fd_used_once(fdinfo_list, errors)
    errors.pop_context()

@extra_check       
def validate_fdinfo_dict(fdinfo_dict, resources, errors_list):
    errors.add_context("validating fdinfo_list")
    for item_tuple in fdinfo_dict:
        errors_list.add_context("fdinfos for {}".format(item_tuple[0]))
        validate_fdinfo_list(item_tuple[1], resources, errors_list)
        errors_list.pop_context()
    errors.pop_context()
    

if __name__ == '__main__':
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    processes = PsList(reader, errors) 
    inventory = Inventory(reader, errors)
    pipes_data_list = FdInfoDict(inventory, processes, reader, errors)
    
    resources = Resources()
    resources.regfiles = RegFilesList(reader, errors)
    resources.pipes = PipesList(reader, errors)
    resources.fifos = FifoList(reader, errors)
    resources.intesk = InetskList(reader, errors)
    validate_fdinfo_dict(pipes_data_list, resources, errors)
#     validate_files_ids(inventory, reader, processes, errors)
    
    errors.show_errors()


    