import os
def check_area_addrs(start_addr, end_addr, errors_list):
    all_ok = True

    if start_addr < 0:
        errors_list.add_error("incorrect start address")
        all_ok = False

    if end_addr < 0:
        errors_list.add_error("incorrect end address")
        all_ok = False

    if end_addr - start_addr <= 0:
        errors_list.add_error("incorrect size area")
        all_ok = False

    return all_ok


#  No validation pgoff, shmid, prot, flags, status, madv, fdflags
def validate_vmas(vmas, errors_list):
    prev_start_addr = 0
    page_size = os.sysconf('SC_PAGE_SIZE')

    for i_vma in range(len(vmas)):
        errors_list.add_context("vma #{0}".format(i_vma))

        check_area_addrs(vmas[i_vma].start, vmas[i_vma].end, errors_list)

        if prev_start_addr >= vmas[i_vma].start:
            errors_list.add_error("vmas is not sorted")

        prev_start_addr = vmas[i_vma].start

        if vmas[i_vma].start % page_size != 0:
            errors_list.add_error("start address vma is not a multiple of {0}".format(page_size))

        for j_vma in range(i_vma + 1, len(vmas)):
            vma_intersects = vmas[j_vma].start <= vmas[i_vma].start < vmas[j_vma].end or \
                             vmas[j_vma].start < vmas[i_vma].end <= vmas[j_vma].end

            if vma_intersects:
                errors_list.add_error("intersects with vma #{0})".format(j_vma))

        errors_list.pop_context()


def check_area_in_vma(vmas, start_addr, end_addr, errors_list):
    i_start_vma = 0
    for i_vma in range(len(vmas)):
        if vmas[i_vma].start <= start_addr < vmas[i_vma].end:
            if vmas[i_vma].start < end_addr <= vmas[i_vma].end:
                return

            i_start_vma = i_vma
            break
    else:
        errors_list.add_error("area is not in vma")

    for i_vma in range(i_start_vma + 1, len(vmas)):
        if vmas[i_vma].end >= end_addr > vmas[i_vma].start == vmas[i_vma - 1].end:
            return
    else:
        errors_list.add_error("area is not in vma")


def validate_mmap_areas(mmap, errors_list):
    vmas = mmap.vmas

    errors_list.add_context("code area")
    if check_area_addrs(mmap.mm_start_code, mmap.mm_end_code, errors_list):
        check_area_in_vma(vmas, mmap.mm_start_code, mmap.mm_end_code, errors_list)
    errors_list.pop_context()

    errors_list.add_context("data area")
    if check_area_addrs(mmap.mm_start_data, mmap.mm_end_data, errors_list):
        check_area_in_vma(vmas, mmap.mm_start_data, mmap.mm_end_data, errors_list)
    errors_list.pop_context()

    errors_list.add_context("stack area")
    if check_area_addrs(mmap.mm_start_stack, mmap.mm_start_stack + 1, errors_list):
        check_area_in_vma(vmas, mmap.mm_start_stack, mmap.mm_start_stack + 1, errors_list)
    errors_list.pop_context()

    errors_list.add_context("brk area")
    if check_area_addrs(mmap.mm_start_brk, mmap.mm_brk, errors_list):
        check_area_in_vma(vmas, mmap.mm_start_brk, mmap.mm_brk, errors_list)
    errors_list.pop_context()

    errors_list.add_context("arg area")
    if check_area_addrs(mmap.mm_arg_start, mmap.mm_arg_end, errors_list):
        check_area_in_vma(vmas, mmap.mm_arg_start, mmap.mm_arg_end, errors_list)
    errors_list.pop_context()

    errors_list.add_context("env area")
    if check_area_addrs(mmap.mm_env_start, mmap.mm_env_end, errors_list):
        check_area_in_vma(vmas, mmap.mm_env_start, mmap.mm_env_end, errors_list)
    errors_list.pop_context()


#  No validation exe_file_id, mm_saved_auxv, aios, dumpable
def validate_mmap_ps(process, errors_list):
    errors_list.add_context("mmap")
    mmap = process.mmap
    vmas = mmap.vmas

    validate_vmas(vmas, errors_list)
    validate_mmap_areas(mmap, errors_list)

    errors_list.pop_context()


def validate_pmap_ps(ind_ps, processes, errors_list, dir_path):
    errors_list.add_context("pmap")

    pmap = processes[ind_ps].pmap
    pages_img_file_name = "pages-{0}.img".format(pmap.pages_id)
    pages_img_path = os.path.join(dir_path, pages_img_file_name)

    page_size = os.sysconf('SC_PAGE_SIZE')
    pages_size = 0
    entryes = pmap.pmap_entryes
    for i_entry in range(len(entryes)):
        errors_list.add_context("page map #{0}".format(i_entry))

        start_addr = entryes[i_entry].vaddr
        end_addr = start_addr + entryes[i_entry].nr_pages * page_size

        for j_entry in range(i_entry + 1, len(entryes)):
            start_addr_other = entryes[j_entry].vaddr
            end_addr_other = start_addr_other + entryes[j_entry].nr_pages * page_size

            pages_intersects = start_addr_other <= start_addr < end_addr_other or \
                               start_addr_other < end_addr <= end_addr_other

            if pages_intersects:
                errors_list.add_error("intersects with page map #{0})".format(j_entry))

        pages_size += end_addr - start_addr

        if not entryes[i_entry].in_parent:
            check_area_in_vma(processes[ind_ps].mmap.vmas, start_addr, end_addr, errors_list)
        else:
            errors_list.add_error("incremental dumps not supported")

        errors_list.pop_context()

    if not os.path.exists(pages_img_path):
        errors_list.add_error("no file {0}".format(pages_img_path))

    if os.path.getsize(pages_img_path) != pages_size:
        errors_list.add_error("wrong size file {0}".format(pages_img_path))

    errors_list.pop_context()


# Warning: False positives.
# Criu store not all pmap.
def check_cow_vma(vma_start, vma_end, node, errors_list):

    if node is None:
        errors_list.add_error("no all pages for vma (addr: {0} - {1})"
                                  .format(vma_start, vma_end))
        return None

    pmap_entryes = node.process.pmap.pmap_entryes
    page_size = os.sysconf('SC_PAGE_SIZE')

    for entry in pmap_entryes:
        start_addr = entry.vaddr
        end_addr = start_addr + entry.nr_pages * page_size

        if vma_start <= start_addr and end_addr <= vma_end:
            if start_addr - vma_start > 0:
                check_cow_vma(vma_start, start_addr, node, errors_list)
            elif vma_end - end_addr > 0:
                check_cow_vma(end_addr, vma_end, node, errors_list)
            break
    else:
        check_cow_vma(vma_start, vma_end, node.parent, errors_list)


def check_cow_vmas(node, errors_list):
    vmas = node.process.mmap.vmas

    errors_list.add_context("Process (pid: {0})".format(node.process.pid))
    for vma in vmas:
        check_cow_vma(vma.start, vma.end, node, errors_list)
    errors_list.pop_context()

    for child in node.children:
        check_cow_vmas(child, errors_list)


def validate_memory(processes, ps_tree, errors_list, dir_path):
    ps_list = processes.process_list

    for i_ps in range(len(ps_list)):
        errors_list.add_context("Process #{0}".format(i_ps))
        validate_mmap_ps(ps_list[i_ps], errors_list)
        validate_pmap_ps(i_ps, ps_list, errors_list, dir_path)
        errors_list.pop_context()

    check_cow_vmas(ps_tree.root, errors_list)