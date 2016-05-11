import sys
from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList
from resgraph.functional_divider import extra_check, enable_extra


from resgraph.elements import PipesDataEntry, PipesDataList
import common_checks

def check_pipe_id(pipe_data, errors):
    if not common_checks.check_is_int(pipe_data.pipe_id):
        errors.add_error("wrong pipe id")
        
def check_bytes(pipe_data, errors):
    if not common_checks.check_is_int(pipe_data.bytes) or \
        common_checks.check_is_negative(pipe_data.bytes):
            errors.add_error("wrong bytes number")
            
def check_size(pipe_data, errors):
    pass

def check_extra(pipe_data, errors):
    if pipe_data.extra != None and len(pipe_data.extra) < pipe_data.bytes:
        errors.add_error("wrong pipe_data data len - smaller than bytes num")
        
def check_single_pipe_data_for_pipe_id(pipe_data_list, errors):
    common_checks.do_check_unique_for_field(pipe_data_list, "pipe_id", errors)

@extra_check
def validate_pipes_data_entry(pipe_data, errors):
    errors.add_context("validating pipe_data_entry")
    check_pipe_id(pipe_data, errors)
    check_bytes(pipe_data, errors)
    check_size(pipe_data, errors)
    check_extra(pipe_data, errors)
    errors.pop_context()
    
@extra_check
def validate_pipes_data_list(pipes_data_list, errors):
    errors.add_context("validating pipes data list")
    for item in pipes_data_list:
        validate_pipes_data_entry(item, errors)
    check_single_pipe_data_for_pipe_id(pipes_data_list, errors)
    errors.pop_context()

if __name__ == "__main__":
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    pipes_data_list = PipesDataList(reader, errors)
    validate_pipes_data_list(pipes_data_list, errors)
    errors.show_errors()
    