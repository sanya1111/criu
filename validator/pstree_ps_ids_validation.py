def get_all_ps_ids(processes):
    pids_list = []
    ppids_list = []
    pgids_list = []
    sids_list = []

    for ps in processes.process_list:
        pids_list.append(ps.pid)
        ppids_list.append(ps.ppid)
        pgids_list.append(ps.pgid)
        sids_list.append(ps.sid)

    return pids_list, ppids_list, pgids_list, sids_list


def check_positiveness(type_pids, pids_list, errors_list):
    for i_pid in range(len(pids_list)):
        if not pids_list[i_pid] > 0 and type_pids != 'ppid':
            context = "Process #{0}".format(i_pid)
            errors_list.add_context(context)
            error_str = "Incorrect {0}({1})".format(type_pids, pids_list[i_pid])
            errors_list.add_error(error_str)
            errors_list.pop_context()


def check_uniqueness_pids(pids_list, errors_list):
    for i_pid in range(len(pids_list)):
        current_pid = pids_list[i_pid]
        num_current_pid = pids_list.count(current_pid)
        if num_current_pid > 1:
            context = "Process #{0}".format(i_pid)
            errors_list.add_context(context)
            error_str = "There are already {0} processes with pid {1}".format(num_current_pid - 1, current_pid)
            errors_list.add_error(error_str)
            errors_list.pop_context()


def find_ps_in_tree(pid, pstree):
    for ps in pstree:
        if pid == ps.pid:
            return True
    else:
        return False


def check_parents(ppids_list, pstree, errors_list):
    for i_ppid in range(len(ppids_list)):
        if ppids_list[i_ppid] != 0:
            if not find_ps_in_tree(ppids_list[i_ppid], pstree):
                context = "Process #{0}".format(i_ppid)
                errors_list.add_context(context)
                error_str = "No parent in three (ppid - {0})".format(ppids_list[i_ppid])
                errors_list.add_error(error_str)
                errors_list.pop_context()


def validate_pids(pids_list, errors_list):
    check_positiveness('pid', pids_list, errors_list)
    check_uniqueness_pids(pids_list, errors_list)


def validate_ppids(ppids_list, ps_tree, errors_list):
    check_positiveness('ppid', ppids_list, errors_list)
    check_parents(ppids_list, ps_tree, errors_list)


def validate_pgids(pgids_list, errors_list):
    check_positiveness('pgid', pgids_list, errors_list)


def validate_sids(sids_list, errors_list):
    check_positiveness('sid', sids_list, errors_list)
    
def validate_ps_ids(processes, ps_tree, errors_list):
    pids, ppids, pgids, sids = get_all_ps_ids(processes)
    validate_pids(pids, errors_list)
    if ps_tree.root:
        validate_ppids(ppids, ps_tree, errors_list)
        validate_pgids(pgids, errors_list)
        validate_sids(sids, errors_list)
    
    