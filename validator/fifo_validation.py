import sys

from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList, RegFilesList
from resgraph.elements import FifoList, PipesDataList
from resgraph.functional_divider import extra_check, enable_extra
import common_checks

def check_id(fifo, errors):
    if not common_checks.check_is_int(fifo.id):
        errors.add_error("wrong id")

def check_pipe_id(fifo, errors):
    if not common_checks.check_is_int(fifo.pipe_id):
        errors.add_error("wrong fifo id")
        
def check_compability_with_data(fifo, fifo_data_list, errors):
    if filter(lambda x : fifo.pipe_id == x.pipe_id, fifo_data_list) == []:
        errors.add_error("no data with pipe_id = {} found".format(fifo.pipe_id))
        
def check_compablity_with_regfiles(fifo, regfiles, errors):
    if filter(lambda x : fifo.id == x.id, regfiles) == []:
        errors.add_error("no regfile with id = {} found".format(fifo.id))
        
def check_single_fifo_for_id(fifo_list, errors):
    common_checks.do_check_unique_for_field(fifo_list, "id", errors)
            
@extra_check
def validate_one_fifo(fifo, fifo_data_list, regfiles, errors):
    errors.add_context("validating fifo")
    check_id(fifo, errors)
    check_pipe_id(fifo, errors)
    check_compability_with_data(fifo, fifo_data_list, errors)
    check_compablity_with_regfiles(fifo, regfiles, errors)
    errors.pop_context()
    
@extra_check
def validate_fifos_list(fifos_list, fifos_data_list, regfiles, errors):
    errors.add_context("validating fifos list")
    for item in fifos_list:
        validate_one_fifo(item, fifos_data_list, regfiles, errors)
    check_single_fifo_for_id(fifos_list, errors)
    errors.pop_context()

if __name__ == '__main__':
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    fifo_list = FifoList(reader, errors)
    fifo_data_list = PipesDataList(reader, errors, True)
    reg_files_list = RegFilesList(reader, errors)
    validate_fifos_list(fifo_list, fifo_data_list, reg_files_list, errors)
    errors.show_errors()