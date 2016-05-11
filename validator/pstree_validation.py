from pstree_memory_validation import validate_memory
from pstree_ps_ids_validation import validate_ps_ids

# TODO implement incremental dump validation once
# resgraph model supports them

# TODO print all addresses and flags in HEX


    

def validate(processes, ps_tree, errors_list, dir_path):
    validate_ps_ids(processes, ps_tree, errors_list)
    validate_memory(processes, ps_tree, errors_list, dir_path)
