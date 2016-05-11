import sys
import os
from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList, PipesList
from resgraph.functional_divider import extra_check, enable_extra


from resgraph.elements import PipesDataEntry, PipesDataList
import common_checks
from posix import O_DIRECT, O_NONBLOCK

'''
LEFT
fown
'''

def check_id(pipe, errors):
    if not common_checks.check_is_int(pipe.id):
        errors.add_error("wrong id")

def check_pipe_id(pipe, errors):
    if not common_checks.check_is_int(pipe.pipe_id):
        errors.add_error("wrong pipe id")
        
def check_flags(pipe, errors):
    if not common_checks.check_open_flags(pipe.flags, [O_DIRECT, O_NONBLOCK]):
        errors.add_error("wrong flags") 

def check_fown(pipe, errors):
    pass

def check_compability_with_data(pipe, pipe_data_list, errors):
    if filter(lambda x : pipe.pipe_id == x.pipe_id, pipe_data_list) == []:
        errors.add_error("no data with pipe_id = {} found".format(pipe.pipe_id))
        
def check_single_pipe_for_id(pipe_list, errors):
    common_checks.do_check_unique_for_field(pipe_list, "id", errors)
    
# def check_exists_pipe_pair(pipe_list, errors):
#     for item in pipe_list:
#         sames = filter(lambda x : item.pipe_id == x.pipe_id, pipe_list)
#         '''
#         need to check that number of pipes with O_RDONLY >= 1
#         and number with O_WRONLY >= 1
#         but we at this moment don't know values of this constanstatns
#         '''
#      
            

@extra_check
def validate_one_pipe(pipe, pipe_data_list, errors):
    errors.add_context("validating pipe")
    check_id(pipe, errors)
    check_pipe_id(pipe, errors)
    check_flags(pipe, errors)
    check_fown(pipe, errors)
    check_compability_with_data(pipe, pipe_data_list, errors)
    errors.pop_context()
    
@extra_check
def validate_pipes_list(pipes_list, pipes_data_list, errors):
    errors.add_context("validating pipes list")
    for item in pipes_list:
        validate_one_pipe(item, pipes_data_list, errors)
    check_single_pipe_for_id(pipes_list, errors)
    errors.pop_context()
        
if __name__ == "__main__":
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    pipes_list = PipesList(reader, errors)
    pipes_data_list = PipesDataList(reader, errors)
    validate_pipes_list(pipes_list, pipes_data_list, errors)
    errors.show_errors()