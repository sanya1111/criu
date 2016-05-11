# TODO connect all the elements of resource graph that have common relations
#      remove connections by indexes in arrays - make model high level and convenient.
# TODO make explicit set of resource graph nodes. Each node is connected with another
#      nodes. Mode each node type and its supplementary classes to node type python file.
# TODO support incremental dumps. We probably need 2 resgraph views: one that hides
#      the fact that dump is incremental another that 
# TODO print all addresses and flags in HEX

import os
import re
from classfieldshelpers import *
from criudumpreader import CriuDumpReader
from errlist import ErrorList
from collections import defaultdict
from resgraph.functional_divider import extra_check, enable_extra

class PsNode:
    def __init__(self, ps=None, parent=None):
        self.process = ps
        self.parent = parent
        self.children = []

    def __iter__(self):
        for child_node in self.children:
            for ps in child_node:
                yield ps
        yield self.process


class PsList:
    def __init__(self, reader, errors_list):
        self.process_list = []
        self.__pstree_file_name = "pstree.img"

        errors_list.add_context("List of processes")
        raw_pslist = reader.read_img(self.__pstree_file_name, errors_list)
        if 'entries' in raw_pslist:
            for raw_psinfo in raw_pslist['entries']:
                ps_context = "process #{0}".format(raw_pslist['entries'].index(raw_psinfo))
                errors_list.add_context(ps_context)

                ps = Process(raw_psinfo, reader, errors_list)
                self.process_list.append(ps)

                errors_list.pop_context()
        else:
            errors_list.add_error("Not entries")

        errors_list.pop_context()

    def show_pslist(self):
        for num_ps in range(len(self.process_list)):
            print "\nProcess #{0}:".format(num_ps)
            self.process_list[num_ps].show_process(2)


class PsTree:
    def __init__(self, processes, errors_list):
        self.root = None

        errors_list.add_context("Tree of processes")
        self.root = PsTree.__create_pstree(processes.process_list, errors_list)
        errors_list.pop_context()

    def __iter__(self):
        for ps in self.root:
            yield ps

    @staticmethod
    def __find_root_ps(ps_list, errors_list):
        roots_list = []
        for ps in ps_list:
            if ps.ppid == 0:
                roots_list.append(ps)

        if roots_list:
            if len(roots_list) > 1:
                errors_list.add_error("Root tree of processes is not one")
            else:
                return roots_list[0]
        else:
            errors_list.add_error("Root tree of processes is not found")

    @staticmethod
    def __find_children_ps(pid, ps_list):
        children = []
        for ps in ps_list:
            if ps.ppid == pid:
                children.append(ps)
        return children

    @staticmethod
    def __create_psnodes(node, ps_list):
        pid = node.process.pid
        children = PsTree.__find_children_ps(pid, ps_list)

        for ps in children:
            new_node = PsNode(ps)
            new_node.parent = node
            PsTree.__create_psnodes(new_node, ps_list)
            node.children.append(new_node)

    @staticmethod
    def __create_pstree(ps_list, errors_list):
        root = None
        ps_root = PsTree.__find_root_ps(ps_list, errors_list)

        if ps_root:
            root = PsNode(ps_root)
            PsTree.__create_psnodes(root, ps_list)
        return root

    @staticmethod
    def __show_edges(node):
        for child in node.children:
            print "ps (pid: {0}) - ps (pid: {1})".format(node.process.pid, child.process.pid)
            PsTree.__show_edges(child)

    def show_pstree(self):
        print "\nEdges tree:"
        PsTree.__show_edges(self.root)

        for ps in self:
            print "\nProcess (pid: {0})".format(ps.pid)
            ps.show_process(2)


class Process:
    def __init__(self, raw_psinfo, reader, errors_list):
        self.pid = None
        self.ppid = None
        self.pgid = None
        self.sid = None
        self.threads = []
        self.mmap = None
        self.pmap = None

        init_items_list = ['pid', 'ppid', 'pgid', 'sid']
        init_items(self, init_items_list, raw_psinfo, errors_list)

        threads_id = get_simple_item(raw_psinfo, 'threads', errors_list)

        for tid in threads_id:
            core_file_name = "core-{0}.img".format(tid)

            context = "thread-{0}".format(tid)
            errors_list.add_context(context)
            raw_trcore = reader.read_img(core_file_name, errors_list)
            if 'entries' in raw_trcore:
                tread = Thread(reader, tid, raw_trcore['entries'][0], errors_list)
                self.threads.append(tread)
            else:
                errors_list.add_error("no entries thread")
                tread = Thread(tid, {}, errors_list)
                self.threads.append(tread)

            errors_list.pop_context()

        mmap_file_name = "mm-{0}.img".format(self.pid)
        raw_mmap = reader.read_img(mmap_file_name, errors_list)
        if 'entries' in raw_mmap:
            self.mmap = MMap(raw_mmap['entries'][0], errors_list)
        else:
            errors_list.add_error("no entries memmory map")

        pmap_file_name = "pagemap-{0}.img".format(self.pid)
        raw_pmap = reader.read_img(pmap_file_name, errors_list)
        if 'entries' in raw_pmap:
            self.pmap = PageMap(raw_pmap['entries'], errors_list)
        else:
            errors_list.add_error("no entries page map")

    def show_process(self, indent_len=0):
        indent = " " * indent_len

        print_items_list = ['pid', 'ppid', 'pgid', 'sid']
        print_items(self, print_items_list, indent)

        if self.threads:
            for num_tr in range(len(self.threads)):
                print "\n{0}Thread #{1}:".format(indent, num_tr)
                self.threads[num_tr].show_thread(indent_len + 2)

        if self.mmap:
            print "\n{0}Memory map:".format(indent)
            self.mmap.show_mmap(indent_len + 2)

        if self.pmap:
            print "\n{0}Page map:".format(indent)
            self.pmap.show_pmap(indent_len + 2)


class Thread:
    def __init__(self, reader, pid, raw_trcore_info, errors_list):
        self.pid = pid
        self.mtype = None
        self.thread_info = None  # optional
        self.tc = None  # optional
        self.ids = None  # optional
        self.thread_core = None  # optional

        if raw_trcore_info:
            self.mtype = get_simple_item(raw_trcore_info, 'mtype', errors_list)

            if 'thread_info' in raw_trcore_info:
                errors_list.add_context("thread_info")
                self.thread_info = ThreadInfo_x86(raw_trcore_info['thread_info'], errors_list)
                errors_list.pop_context()

            if 'tc' in raw_trcore_info:
                errors_list.add_context("tc")
                self.tc = TC(raw_trcore_info['tc'], errors_list)
                errors_list.pop_context()

            if 'ids' in raw_trcore_info:
                errors_list.add_context("ids")
                self.ids = Ids(None, None, raw_trcore_info['ids'], errors_list)
                errors_list.pop_context()

            if 'thread_core' in raw_trcore_info:
                errors_list.add_context("thread_core")
                self.thread_core = ThreadCore(raw_trcore_info['thread_core'], errors_list)
                errors_list.pop_context()
        
        if self.ids == None:
            self.ids = Ids(reader, pid, None, errors_list)
        

    def show_thread(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['pid', 'mtype']
        print_items(self, print_items_list, indent)

        if self.thread_info:
            print "{0}thread info:".format(indent)
            self.thread_info.show_thread(indent_len + 2)

        if self.tc:
            print "{0}tc:".format(indent)
            self.tc.show_tc(indent_len + 2)

        if self.ids:
            print "{0}ids:".format(indent)
            self.ids.show_ids(indent_len + 2)

        if self.thread_core:
            print "{0}thread_core:".format(indent)
            self.thread_core.show_trcore(indent_len + 2)


class GPRegs_x86:
    def __init__(self, raw_gpregs_info, errors_list):
        self.r15 = None
        self.r14 = None
        self.r13 = None
        self.r12 = None
        self.bp = None
        self.r11 = None
        self.r10 = None
        self.r9 = None
        self.r8 = None
        self.ax = None
        self.cx = None
        self.dx = None
        self.si = None
        self.di = None
        self.orig_ax = None
        self.ip = None
        self.cs = None
        self.flags = None
        self.sp = None
        self.fs_base = None
        self.gs_base = None
        self.ds = None
        self.es = None
        self.fs = None
        self.gs = None

        init_items_list = ['r15', 'r14', 'r13', 'r12', 'bp', 'r11', 'r10', 'r9',
                           'r8', 'ax', 'cx', 'dx', 'si', 'di', 'orig_ax', 'ip',
                           'cs', 'flags', 'sp', 'fs_base', 'gs_base', 'ds',
                           'es', 'fs', 'gs']
        init_items(self, init_items_list, raw_gpregs_info, errors_list)

    def show_gpregs(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['r15', 'r14', 'r13', 'r12', 'bp', 'r11', 'r10', 'r9',
                            'r8', 'ax', 'cx', 'dx', 'si', 'di', 'orig_ax', 'ip',
                            'cs', 'flags', 'sp', 'fs_base', 'gs_base', 'ds',
                            'es', 'fs', 'gs']
        print_items(self, print_items_list, indent)


class XSave:
    def __init__(self, raw_xsave_info, errors_list):
        self.xstate_bv = None
        self.ymmh_space = None

        init_items_list = ['xstate_bv', 'ymmh_space']
        init_items(self, init_items_list, raw_xsave_info, errors_list)

    def show_xsave(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['xstate_bv', 'ymmh_space']
        print_items(self, print_items_list, indent)


class FPRegs_x86:
    def __init__(self, raw_fpregs_info, errors_list):
        self.cwd = None
        self.swd = None
        self.twd = None
        self.fop = None
        self.rip = None
        self.rdp = None
        self.mxcsr = None
        self.mxcsr_mask = None
        self.st_space = None
        self.xmm_space = None
        self.xsave = None  # optional

        self.padding = raw_fpregs_info.get('padding')  # Unused, but present for backward compatibility

        init_items_list = ['cwd', 'swd', 'twd', 'fop', 'rip', 'rdp', 'mxcsr',
                           'mxcsr_mask', 'st_space', 'xmm_space']
        init_items(self, init_items_list, raw_fpregs_info, errors_list)

        if 'xsave' in raw_fpregs_info:
            self.xsave = XSave(raw_fpregs_info['xsave'], errors_list)

    def show_fpregs(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['cwd', 'swd', 'twd', 'fop', 'rip', 'rdp', 'mxcsr',
                            'mxcsr_mask', 'st_space', 'xmm_space', 'padding']
        print_items(self, print_items_list, indent)

        if self.xsave:
            self.xsave.show_xsave(indent_len + 2)


class ThreadInfo_x86:
    def __init__(self, raw_thread_info, errors_list):
        self.clear_tid_addr = get_simple_item(raw_thread_info, 'clear_tid_addr', errors_list)
        self.gpregs = None
        self.fpregs = None

        errors_list.add_context('gpregs')
        if 'gpregs' in raw_thread_info:
            self.gpregs = GPRegs_x86(raw_thread_info['gpregs'], errors_list)
        else:
            errors_list.add_error("no item gpregs")
        errors_list.pop_context()

        errors_list.add_context('fpregs')
        if 'fpregs' in raw_thread_info:
            self.fpregs = FPRegs_x86(raw_thread_info['fpregs'], errors_list)
        else:
            errors_list.add_error("no item fpregs")
        errors_list.pop_context()

    def show_thread(self, indent_len=0):
        indent = " " * indent_len
        print "{0}clear_tid_addr - {1}".format(indent, self.clear_tid_addr)

        if self.gpregs is not None:
            print "{0}gpregs:".format(indent)
            self.gpregs.show_gpregs(indent_len + 2)
        else:
            print "{0}gpregs - {1}".format(indent, self.gpregs)

        if self.fpregs is not None:
            print "{0}fpregs:".format(indent)
            self.fpregs.show_fpregs(indent_len + 2)
        else:
            print "{0}fpregs - {1}".format(indent, self.fpregs)


class RLimits:
    def __init__(self, raw_rlimits_info, errors_list):
        self.cur = None
        self.max = None

        init_items_list = ['cur', 'max']
        init_items(self, init_items_list, raw_rlimits_info, errors_list)

    def show_rlimits(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['cur', 'max']
        print_items(self, print_items_list, indent)


class RLimitsList:
    def __init__(self, raw_rlimits_list_info, errors_list):
        self.rlimits_list = []

        if 'rlimits' in raw_rlimits_list_info:
            for raw_rlinfo in raw_rlimits_list_info['rlimits']:
                rl_context = "rlimits #{0}".format(
                    raw_rlimits_list_info['rlimits'].index(raw_rlinfo))
                errors_list.add_context(rl_context)

                rlimits = RLimits(raw_rlinfo, errors_list)
                self.rlimits_list.append(rlimits)

                errors_list.pop_context()
        else:
            errors_list.add_error("no rlimits")

    def show_rlimits_list(self, indent_len=0):
        indent = " " * indent_len
        for num_rlimits in range(len(self.rlimits_list)):
            print "{0}rlimits #{1}:".format(indent, num_rlimits)
            self.rlimits_list[num_rlimits].show_rlimits(indent_len + 2)


class SigInfo:
    def __init__(self, raw_sig_info, errors_list):
        self.siginfo = None
        init_items_list = ['siginfo']
        init_items(self, init_items_list, raw_sig_info, errors_list)

    def show_siginfo(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['siginfo']
        print_items(self, print_items_list, indent)


class SigQueue:
    def __init__(self, raw_sigqueue_info, errors_list):
        self.signals_list = []

        if 'signals' in raw_sigqueue_info:
            for raw_siginfo in raw_sigqueue_info['signals']:
                sig_context = "signal #{0}".format(
                    raw_sigqueue_info['signals'].index(raw_siginfo))
                errors_list.add_context(sig_context)

                signal = SigInfo(raw_siginfo, errors_list)
                self.signals_list.append(signal)

                errors_list.pop_context()

        else:
            errors_list.add_error("no signals")

    def show_sig_queue(self, indent_len=0):
        for num_sig in range(len(self.signals_list)):
            print "signal #{0}:".format(num_sig)
            self.signals_list[num_sig].show_siginfo(indent_len + 2)


class ITimer:
    def __init__(self, raw_itimer_info, errors_list):
        self.isec = None
        self.iusec = None
        self.vsec = None
        self.vusec = None

        init_items_list = ['isec', 'iusec', 'vsec', 'vusec']
        init_items(self, init_items_list, raw_itimer_info, errors_list)

    def show_itimer(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['isec', 'iusec', 'vsec', 'vusec']
        print_items(self, print_items_list, indent)


class PosixTimer:
    def __init__(self, raw_ptimer_info, errors_list):
        self.it_id = None
        self.clock_id = None
        self.si_signo = None
        self.it_sigev_notify = None
        self.sival_ptr = None
        self.overrun = None
        self.isec = None
        self.insec = None
        self.vsec = None
        self.vnsec = None

        init_items_list = ['it_id', 'clock_id', 'si_signo', 'it_sigev_notify', 'sival_ptr',
                           'overrun', 'isec', 'insec', 'vsec', 'vnsec']
        init_items(self, init_items_list, raw_ptimer_info, errors_list)

    def show_ptimer(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['it_id', 'clock_id', 'si_signo', 'it_sigev_notify', 'sival_ptr',
                            'overrun', 'isec', 'insec', 'vsec', 'vnsec']
        print_items(self, print_items_list, indent)


class Timers:
    def __init__(self, raw_timers_info, errors_list):
        self.real = None
        self.virt = None
        self.prof = None

        errors_list.add_context("real")
        if 'real' in raw_timers_info:
            self.real = ITimer(raw_timers_info['real'], errors_list)
        else:
            errors_list.add_error("no item real")
        errors_list.pop_context()

        errors_list.add_context("virt")
        if 'virt' in raw_timers_info:
            self.virt = ITimer(raw_timers_info['virt'], errors_list)
        else:
            errors_list.add_error("no item virt")
        errors_list.pop_context()

        errors_list.add_context("prof")
        if 'prof' in raw_timers_info:
            self.prof = ITimer(raw_timers_info['prof'], errors_list)
        else:
            errors_list.add_error("no item prof")
        errors_list.pop_context()

        errors_list.add_context("posix")
        if 'posix' in raw_timers_info:
            self.posix_timers = []
            for raw_ptimer in raw_timers_info['posix']:
                ptimer = PosixTimer(raw_ptimer, errors_list)
                self.posix_timers.append(ptimer)
        errors_list.pop_context()

    def show_timers(self, indent_len=0):
        indent = " " * indent_len
        if self.real is not None:
            print "{0}real".format(indent)
            self.real.show_itimer(indent_len + 2)
        else:
            print "{0}real - {1}".format(indent, self.real)

        if self.virt is not None:
            print "{0}virt".format(indent)
            self.virt.show_itimer(indent_len + 2)
        else:
            print "{0}virt - {1}".format(indent, self.real)

        if self.prof is not None:
            print "{0}prof".format(indent)
            self.prof.show_itimer(indent_len + 2)
        else:
            print "{0}prof - {1}".format(indent, self.real)

        if 'posix_timers' in self.__dict__:
            print "{0}posix".format(indent)
            for num_pt in range(len(self.posix_timers)):
                print "posix #{0}".format(num_pt)
                self.posix_timers[num_pt].show_ptimer(indent_len + 2)


class TC:
    def __init__(self, raw_tc_info, errors_list):
        self.task_state = None
        self.exit_code = None
        self.personality = None
        self.flags = None
        self.blk_sigset = None
        self.comm = None
        self.cg_set = raw_tc_info.get('cg_set')  # optional
        self.seccomp_mode = raw_tc_info.get('seccomp_mode')  # optional
        self.timers = None  # optional
        self.rlimits = None  # optional
        self.signals_s = None  # optional

        init_items_list = ['task_state', 'exit_code', 'personality', 'flags',
                           'blk_sigset', 'comm']
        init_items(self, init_items_list, raw_tc_info, errors_list)

        if 'timers' in raw_tc_info:
            errors_list.add_context("timers")
            self.timers = Timers(raw_tc_info['timers'], errors_list)
            errors_list.pop_context()

        if 'rlimits' in raw_tc_info and len(raw_tc_info['rlimits']) != 0:
            errors_list.add_context("rlimits")
            self.rlimits = RLimitsList(raw_tc_info['rlimits'], errors_list)
            errors_list.pop_context()

        if 'signals_s' in raw_tc_info and len(raw_tc_info['signals_s']) != 0:
            errors_list.add_context("signals_s")
            self.signals_s = SigQueue(raw_tc_info['signals_s'], errors_list)
            errors_list.pop_context()

    def show_tc(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['task_state', 'exit_code', 'personality', 'flags',
                            'blk_sigset', 'comm', 'cg_set', 'seccomp_mode']
        print_items(self, print_items_list, indent)

        if self.timers:
            print "{0}timers:".format(indent)
            self.timers.show_timers(indent_len + 2)

        if self.rlimits:
            print "{0}rlimits:".format(indent)
            self.rlimits.show_rlimits_list(indent_len + 2)

        if self.signals_s:
            print "{0}signals_s:".format(indent)
            self.signals_s.show_sig_queue(indent_len + 2)


class Ids:
    def __init__(self, reader, pid, raw_ids_info, errors_list):
        self.vm_id = None
        self.files_id = None
        self.fs_id = None
        self.sighand_id = None
        self.pid_ns_id = None  # optional
        self.net_ns_id = None  # optional
        self.ipc_ns_id = None  # optional
        self.uts_ns_id = None  # optional
        self.mnt_ns_id = None  # optional
        self.user_ns_id = None  # optional
        
        if pid != None:
            assigned_ids_img = "ids-{}.img".format(pid)
            errors_list.add_context("reading {0}".format(assigned_ids_img))
            raw_ids_info = reader.read_img(assigned_ids_img, errors_list)
            if not "magic" in raw_ids_info or raw_ids_info.get("magic") != "IDS":
                errors_list.add_error("bad magic")
            elif not "entries" in raw_ids_info:
                errors_list.add("no entries")
            else:
                raw_ids_info = raw_ids_info["entries"][0] 
            errors_list.pop_context()

        init_items_list = ['vm_id', 'files_id', 'fs_id', 'sighand_id']
        init_items(self, init_items_list, raw_ids_info, errors_list)

        init_optional_items_list = ['pid_ns_id', 'net_ns_id', 'ipc_ns_id',
                                    'uts_ns_id', 'mnt_ns_id', 'user_ns_id']
        init_optional_items(self, init_optional_items_list, raw_ids_info, errors_list)
        
        

    def show_ids(self, indent_len=0):
        indent = " " * indent_len

        print_items_list = ['vm_id', 'files_id', 'fs_id', 'sighand_id']
        print_items(self, print_items_list, indent)

        print_optional_items_list = ['pid_ns_id', 'net_ns_id', 'ipc_ns_id',
                                     'uts_ns_id', 'mnt_ns_id', 'user_ns_id']
        print_optional_items(self, print_optional_items_list, indent)
     


class Sas:
    def __init__(self, raw_sas_info, errors_list):
        self.ss_sp = None
        self.ss_size = None
        self.ss_flags = None

        init_items_list = ['ss_sp', 'ss_size', 'ss_flags']
        init_items(self, init_items_list, raw_sas_info, errors_list)

    def show_sas(self, indent_len=0):
        indent = " " * indent_len

        print_items_list = ['ss_sp', 'ss_size', 'ss_flags']
        print_items(self, print_items_list, indent)


class ThreadCore:
    def __init__(self, raw_trcore_info, errors_list):
        self.futex_rla = None
        self.futex_rla_len = None
        self.sched_nice = None  # optional
        self.sched_policy = None  # optional
        self.sched_prio = None  # optional
        self.blk_sigset = None  # optional
        self.pdeath_sig = None  # optional
        self.sas = None  # optional
        self.signals_p = None  # optional

        init_items_list = ['futex_rla', 'futex_rla_len']
        init_items(self, init_items_list, raw_trcore_info, errors_list)

        init_optional_items_list = ['sched_nice', 'sched_policy', 'sched_prio',
                                    'blk_sigset', 'pdeath_sig']
        init_optional_items(self, init_optional_items_list, raw_trcore_info, errors_list)

        if 'sas' in raw_trcore_info:
            errors_list.add_context("sas")
            self.sas = Sas(raw_trcore_info['sas'], errors_list)
            errors_list.pop_context()

        if 'signals_p' in raw_trcore_info and len(raw_trcore_info['signals_p']) != 0:
            errors_list.add_context("signals_p")
            self.signals_p = SigQueue(raw_trcore_info['signals_p'], errors_list)
            errors_list.pop_context()

    def show_trcore(self, indent_len=0):
        indent = " " * indent_len

        print_items_list = ['futex_rla', 'futex_rla_len']
        print_items(self, print_items_list, indent)

        print_optional_items_list = ['sched_nice', 'sched_policy', 'sched_prio',
                                     'blk_sigset', 'pdeath_sig']
        print_optional_items(self, print_optional_items_list, indent)

        if self.sas:
            print "{0}sas:".format(indent)
            self.sas.show_sas(indent_len + 2)

        if self.signals_p:
            print "{0}signals_p:".format(indent)
            self.signals_p.show_sig_queue(indent_len + 2)


class VMA:
    def __init__(self, raw_vma_info, errors_list):
        self.start = None
        self.end = None
        self.pgoff = None
        self.shmid = None
        self.prot = None
        self.flags = None
        self.status = None
        self.madv = None  # optional
        self.fdflags = None  # optional

        init_items_list = ['start', 'end', 'pgoff', 'shmid', 'prot', 'flags', 'status']
        init_items(self, init_items_list, raw_vma_info, errors_list)

        init_optional_items_list = ['madv', 'fdflags']
        init_optional_items(self, init_optional_items_list, raw_vma_info, errors_list)

    def show_vma(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['start', 'end', 'pgoff', 'shmid', 'prot', 'flags', 'status']
        print_items(self, print_items_list, indent)

        print_optional_items_list = ['madv', 'fdflags']
        print_optional_items(self, print_optional_items_list, indent)


class AIO:
    def __init__(self, raw_aio_info, errors_list):
        self.id = None
        self.nr_req = None
        self.ring_len = None

        init_items_list = ['id', 'nr_req', 'ring_len']
        init_items(self, init_items_list, raw_aio_info, errors_list)

    def show_aio(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['id', 'nr_req', 'ring_len']
        print_items(self, print_items_list, indent)


class MMap:
    def __init__(self, raw_mm_info, errors_list):
        self.mm_start_code = None
        self.mm_end_code = None
        self.mm_start_data = None
        self.mm_end_data = None
        self.mm_start_stack = None
        self.mm_start_brk = None
        self.mm_brk = None
        self.mm_arg_start = None
        self.mm_arg_end = None
        self.mm_env_start = None
        self.mm_env_end = None
        self.exe_file_id = None
        self.mm_saved_auxv = []
        self.vmas = []
        self.aios = []
        self.dumpable = []  # optional

        if 'dumpable' in raw_mm_info:
            self.dumpable = raw_mm_info['dumpable']

        init_items_list = ['mm_start_code', 'mm_end_code', 'mm_start_data', 'mm_end_data',
                           'mm_start_stack', 'mm_start_brk', 'mm_brk', 'mm_arg_start',
                           'mm_arg_end', 'mm_env_start', 'mm_env_end', 'exe_file_id']
        init_items(self, init_items_list, raw_mm_info, errors_list)

        init_optional_items_list = ['mm_saved_auxv']  # repeated
        init_optional_items(self, init_optional_items_list, raw_mm_info, errors_list)

        if 'vmas' in raw_mm_info:
            for raw_vma_info in raw_mm_info['vmas']:
                context = "vma #{0}".format(raw_mm_info['vmas'].index(raw_vma_info))
                errors_list.add_context(context)

                vma = VMA(raw_vma_info, errors_list)
                self.vmas.append(vma)

                errors_list.pop_context()

        if 'aios' in raw_mm_info:
            for raw_aio_info in raw_mm_info['aios']:
                context = "aio #{0}".format(raw_mm_info['aios'].index(raw_aio_info))
                errors_list.add_context(context)

                aio = AIO(raw_aio_info, errors_list)
                self.aios.append(aio)

                errors_list.pop_context()

    def show_mmap(self, indent_len=0):
        indent = " " * indent_len

        print_items_list = ['mm_start_code', 'mm_end_code', 'mm_start_data', 'mm_end_data',
                            'mm_start_stack', 'mm_start_brk', 'mm_brk', 'mm_arg_start',
                            'mm_arg_end', 'mm_env_start', 'mm_env_end', 'exe_file_id',
                            'mm_saved_auxv']
        print_items(self, print_items_list, indent)

        if self.vmas:
            for num_vma in range(len(self.vmas)):
                print "\n{0}Vma #{1}:".format(indent, num_vma)
                self.vmas[num_vma].show_vma(indent_len + 2)

        if self.aios:
            for num_aio in range(len(self.aios)):
                print "\n{0}Aio #{1}:".format(indent, num_aio)
                self.aios[num_aio].show_aio(indent_len + 2)


class PageMapEntry:
    def __init__(self, raw_pmentry_info, errors_list):
        self.vaddr = None
        self.nr_pages = None
        self.in_parent = None  # optional

        init_items_list = ['vaddr', 'nr_pages']
        init_items(self, init_items_list, raw_pmentry_info, errors_list)

        init_optional_items_list = ['in_parent']
        init_optional_items(self, init_optional_items_list, raw_pmentry_info, errors_list)

    def show_pmentry(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['vaddr', 'nr_pages']
        print_items(self, print_items_list, indent)

        if self.in_parent:
            print_optional_items_list = ['in_parent']
            print_optional_items(self, print_optional_items_list, indent)


class PageMap:
    def __init__(self, raw_pm_info, errors_list):
        self.pages_id = None
        self.pmap_entryes = []

        init_items_list = ['pages_id']
        init_items(self, init_items_list, raw_pm_info[0], errors_list)

        for i in range(1, len(raw_pm_info)):
            context = "page map #{0}".format(i - 1)
            errors_list.add_context(context)

            pm = PageMapEntry(raw_pm_info[i], errors_list)
            self.pmap_entryes.append(pm)

            errors_list.pop_context()

    def show_pmap(self, indent_len=0):
        indent = " " * indent_len
        print_items_list = ['pages_id']
        print_items(self, print_items_list, indent)

        for i in range(len(self.pmap_entryes)):
            print "\n{0}Page map #{1}:".format(indent, i)
            self.pmap_entryes[i].show_pmentry(indent_len + 2)


class Inventory:
    def __init__(self, reader, errors_list):
        self.__inventory_image_name = "inventory.img"
        
        self.img_version = None
        self.fdinfo_per_id = None #optional than
        self.root_ids  = None;
        self.ns_per_id  = None;
        self.root_cg_set = None;
        self.lsmtype = None;
        errors_list.add_context("inventory reading")
        
        raw_data = reader.read_img(self.__inventory_image_name, errors_list)
        if "magic" in raw_data and raw_data.get("magic") == "INVENTORY":
            if 'entries' in raw_data:
                init_items_list = ['img_version']
                init_items(self, init_items_list, raw_data['entries'][0], errors_list)
        
                init_optional_items_list = ['fdinfo_per_id', 'root_ids', 'ns_per_id', 'root_cg_set', 'lsmtype'];
                init_optional_items(self, init_optional_items_list, raw_data['entries'][0], errors_list)
            else:
                errors_list.add_error("no entries found")
        else:
            errors_list.add_error("WRONG MAGIC")

        errors_list.pop_context()
        
class FdInfoEntry:
    def __init__(self, raw_data, errors_list):
        self.id = None
        self.flags = None
        self.type = None
        self.fd = None
        init_items_list = ['id', 'flags', 'type', 'fd']
        init_items(self, init_items_list, raw_data, errors_list)
    
    def __str__(self):
        return "Fdinfo : id = {0}, flags = {1}, type = {2}, fd = {3}".format(self.id, self.flags, self.type, self.fd)
        
class FdinfoList:
    def __init__(self, img, reader, errors_list):
        self.entries = []
        errors_list.add_context('loading fdinfo image {0}'.format(img))
        raw_data = reader.read_img(img, errors_list, False)
        if raw_data == {}:
            return
        if "magic" in raw_data and raw_data.get("magic") == "FDINFO":
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    errors_list.add_context("trying to load entry {}".format(entry))
                    self.entries.append(FdInfoEntry(entry, errors_list))
                    errors_list.pop_context()
            else:
                errors_list.add_error("no entries found")
        else:
            errors_list.add_error("WRONG MAGIC")
        errors_list.pop_context()
        
    def __iter__(self):
        for item in self.entries:
            yield item
        
class FdInfoDict:
    @extra_check
    def check_no_other_fdinfo_images_exists(self, reader, errors_list):
        for file in os.listdir(reader.dir_img_path):
            if os.path.isfile(os.path.join(reader.dir_img_path, file)):
                m = re.match("^fdinfo-(\d+).img$", file)
                if m != None:
                    if not self.dict.has_key(int(m.group(1))):
                        errors_list.add_error("unused {} image".format(file))
                
    def __init__(self, inventory, processes, reader, errors_list):
        errors_list.add_context("initing fdinfo dictionary")
        self.dict = {}
        if inventory.fdinfo_per_id:
            for task in processes.process_list:
                for thread in task.threads:
                    if thread.ids != None:
                        file_id = thread.ids.files_id
                        if not file_id in self.dict:
                            self.dict[file_id] = FdinfoList("fdinfo-{}.img".format(file_id), reader, errors_list)
        else:
            #TODO under the question, what we should do in this case, in the first view inventory.fdinfo_per_id is always true
            pass
        
        self.check_no_other_fdinfo_images_exists(reader, errors_list)
        errors_list.pop_context()
                            
    def __iter__(self):
        for item in self.dict.items():
            yield item
            
class RegFileEntry:
    def __init__(self, raw_data, errors_list):
        self.id = None
        self.flags = None
        self.pos = None
        self.fown= None
        self.name = None
        
        
        init_items_list = ['id', 'flags', 'pos', 'fown', 'name']
        init_items(self, init_items_list, raw_data, errors_list)
        
        self.mnt_id = None
        self.size = None
        self.ext = None
        
        init_optional_items_list = ['mnt_id', 'size', 'ext'];
        init_optional_items(self, init_optional_items_list, raw_data, errors_list)
    
    def __str__(self):
        return "RegFileEntry : id = {0}, flags = {1}, pos = {2}, fown = {3}, name={4}, mnt_id={5},size={6}, ext={7}".format(
                self.id, self.flags, self.pos, self.fown, self.name, self.mnt_id, self.size, self.ext)
    
            
class RegFilesList:
    def __init__(self, reader, errors_list):
        self.list = []
        reg_img = "reg-files.img"
        
        errors_list.add_context("reading regfiles image")
        raw_data =  reader.read_img(reg_img, errors_list)
        if "magic" in raw_data and raw_data.get("magic") == "REG_FILES":
            if 'entries' in raw_data:
                for entry in raw_data['entries']:
                    self.list.append(RegFileEntry(entry, errors_list))
            else:
                errors_list.add_error("no entries found")
        else:
            errors_list.add_error("WRONG MAGIC")
        errors_list.pop_context()
    
    def __iter__(self):
        for item in self.list:
            yield item
        
class MntPointEntry:
    @extra_check
    def __init__(self, raw_data, errors_list):
        self.fstype = None
        self.mnt_id = None
        self.root_dev = None
        self.parent_mnt_id = None
        self.flags = None
        self.root = None
        self.mountpoint = None
        self.source = None
        self.options = None
        init_items_list = ['fstype', 'mnt_id', 'root_dev', 'parent_mnt_id', 'flags', 'root', 'mountpoint', 'source', 'options']
        init_items(self, init_items_list, raw_data, errors_list)
        
        self.shared_id = None
        self.master_id = None
        self.with_plugin = None
        self.ext_mount = None
        self.fsname = None
        self.internal_sharing = None
        self.deleted = None
        self.sb_flags = None
        init_optional_items_list = ['shared_id', 'master_id', 'with_plugin', 'ext_mount', 'fsname', 'internal_sharing', 'deleted', 'sb_flags']
        init_optional_items(self, init_optional_items_list, raw_data, errors_list)
        
class MntPointList:
    @extra_check
    def __init__(self, id, reader, errors_list):
        self.items = []
        
        img_name = "mountpoints-{}.img".format(id)
        
        errors_list.add_context("loading mntpoint list")
        raw_data = reader.read_img(img_name, errors_list)
        if "magic" in raw_data and raw_data.get("magic") == "MNTS":
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    errors_list.add_context("trying to load entry {}".format(entry))
                    self.items.append(MntPointEntry(entry, errors_list))
                    errors_list.pop_context()
            else:
                errors_list.add_error("no entries found")
        else:
            errors_list.add_error("WRONG MAGIC")
        errors_list.pop_context()
        
    def __iter__(self):
        for item in self.items:
            yield item
            
class MntPointDict():
    def check_no_other_fdinfo_images_exists(self, reader, errors_list):
        for file in os.listdir(reader.dir_img_path):
            if os.path.isfile(os.path.join(reader.dir_img_path, file)):
                m = re.match("^mountpoints-(\d+).img$", file)
                if m != None:
                    if not self.dict.has_key(int(m.group(1))):
                        errors_list.add_error("unused {} image".format(file))
                        
    def do_init(self, parent_mnt_nsid, node, reader, errors_list):
        mnt_nsid = node.process.threads[0].ids.mnt_ns_id
        if mnt_nsid != None:
            if mnt_nsid != parent_mnt_nsid and not mnt_nsid in self.dict:
                self.dict[mnt_nsid] = MntPointList(mnt_nsid, reader, errors_list) 
            for child in node.children: 
                self.do_init(mnt_nsid, child, reader, errors_list)
       
    @extra_check     
    def __init__(self, inventory, pstree, reader, errors_list):
        self.dict = {}
        errors_list.add_context("loading mntpoint dict")
        if inventory.ns_per_id and inventory.root_ids != None:
            self.do_init(inventory.root_ids["mnt_ns_id"], pstree.root, reader, errors_list)
        else:
            pass
        
        self.check_no_other_fdinfo_images_exists(reader, errors_list)
        errors_list.pop_context()
        
    def __iter__(self):
        for item in self.dict.items():
            yield item
    
class PipesDataEntry:
    def __init__(self, raw_data, errors):
        self.pipe_id = None
        self.bytes = None
        
        self.size = None
        self.extra = None
        
        errors.add_context("trying to load new pipes_data_entry")
        init_items_list = ['pipe_id', 'bytes']
        init_items(self, init_items_list, raw_data, errors)
        
        init_optional_items_list = ['size', 'extra']
        init_optional_items(self, init_optional_items_list, raw_data, errors)
        
        errors.pop_context()
        
class PipesDataList:
    def __init__(self, reader, errors, fifo=False):
        self.items = []
        
        img = None
        magic = None
        if fifo:
            img = 'fifo-data.img'
            magic = 'FIFO_DATA'
        else:
            img = 'pipes-data.img'
            magic = 'PIPES_DATA'
        errors.add_context("loading pipes_data list")
        raw_data = reader.read_img(img, errors)
        if "magic" in raw_data and raw_data.get("magic") == magic:
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    self.items.append(PipesDataEntry(entry, errors))
            else:
                errors.add_error("no entries found")
        else:
            errors.add_error("WRONG MAGIC")
        errors.pop_context()
        
    def __iter__(self):
        for item in self.items:
            yield item
            
class PipesEntry:
    def __init__(self, raw_data, errors):
        self.id = None
        self.pipe_id = None;
        self.flags = None
        self.fown = None
        
        errors.add_context("loading pipes entry")
        init_items_list = ['id', 'pipe_id', 'flags', 'fown']
        init_items(self, init_items_list, raw_data, errors)
        errors.pop_context()
        
class PipesList:
    def __init__(self, reader, errors):
        self.items = []
        
        img = 'pipes.img'
        errors.add_context("loading pipes list")
        raw_data = reader.read_img(img, errors)
        if "magic" in raw_data and raw_data.get("magic") == "PIPES":
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    self.items.append(PipesEntry(entry, errors))
            else:
                errors.add_error("no entries found")
        else:
            errors.add_error("WRONG MAGIC")
        errors.pop_context()
        
    def __iter__(self):
        for item in self.items:
            yield item
            
class FifoEntry:
    def __init__(self, raw_data, errors):
        self.id = None
        self.pipe_id = None;
        
        errors.add_context("loading fifo entry")
        init_items_list = ['id', 'pipe_id']
        init_items(self, init_items_list, raw_data, errors)
        errors.pop_context()
        
class FifoList:
    def __init__(self, reader, errors):
        self.items = []
        
        img = 'fifo.img'
        errors.add_context("loading fifos list")
        raw_data = reader.read_img(img, errors)
        if "magic" in raw_data and raw_data.get("magic") == "FIFO":
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    self.items.append(FifoEntry(entry, errors))
            else:
                errors.add_error("no entries found")
        else:
            errors.add_error("WRONG MAGIC")
        errors.pop_context()
        
    def __iter__(self):
        for item in self.items:
            yield item
            
class InetskEntry:
    '''
    required uint32            id        =  1;
    required uint32            ino        =  2;
    required uint32            family        =  3;
    required uint32            type        =  4;
    required uint32            proto        =  5;
    required uint32            state        =  6;
    required uint32            src_port    =  7;
    required uint32            dst_port    =  8;
    required uint32            flags        =  9 [(criu).hex = true];
    required uint32            backlog        = 10;

    repeated uint32            src_addr    = 11 [(criu).ipadd = true];
    repeated uint32            dst_addr    = 12 [(criu).ipadd = true];

    required fown_entry        fown        = 13;
    required sk_opts_entry        opts        = 14;
    
    
    optional bool            v6only        = 15;
    optional ip_opts_entry        ip_opts        = 16;

    /* for ipv6, we need to send the ifindex to bind(); we keep the ifname
     * here and convert it on restore */
    optional string            ifname        = 17;
    '''
    def __init__(self, raw_data, errors):
        self.id = None
        self.ino = None;
        self.family = None
        self.type = None
        self.proto = None
        self.state = None
        self.src_port = None
        self.dst_port = None
        self.flags = None
        self.backlog = None
        self.src_addr = None
        self.dst_addr = None
        self.fown = None
        self.opts = None
        
        self.v6only = None
        self.ip_opts = None
        self.ifname = None
        
        
        
        errors.add_context("loading inetsk entry")
        init_items_list = ['id', 'ino', 'family', 'type', 'proto', 'state', 'src_port', 'dst_port', \
                           'flags', 'backlog', 'src_addr', 'dst_addr', 'fown', 'opts']
        init_items(self, init_items_list, raw_data, errors)
        
        init_optional_items_list = ['v6only', 'ip_opts', 'ifname']
        init_optional_items(self, init_optional_items_list, raw_data, errors)
        errors.pop_context()
        
class InetskList:
    def __init__(self, reader, errors):
        self.items = []
        
        img = 'inetsk.img'
        errors.add_context("loading inetssk list")
        raw_data = reader.read_img(img, errors)
        if "magic" in raw_data and raw_data.get("magic") == "INETSK":
            if 'entries' in raw_data:
                for entry in raw_data["entries"]:
                    self.items.append(InetskEntry(entry, errors))
            else:
                errors.add_error("no entries found")
        else:
            errors.add_error("WRONG MAGIC")
        errors.pop_context()
        
    def __iter__(self):
        for item in self.items:
            yield item       


        
class Resources:
    def __init__(self):
        self.regfiles = None
        self.mntpoints = None
        self.pipes = None
        self.fifos = None