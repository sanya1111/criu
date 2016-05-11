import sys

from resgraph import PsList, ErrorList, CriuDumpReader, Inventory, FdinfoList, RegFilesList
from resgraph.elements import FifoList, PipesDataList, InetskList
from resgraph.functional_divider import extra_check, enable_extra
import common_checks
import socket
import os
from _socket import AF_INET


def check_id(inesk, errors):
    common_checks.check_is_int(inesk.id)
    
def check_ino(inesk, errors):
    common_checks.check_is_int(inesk.ino)
    
'''
All next info gets from sk-inet.c
'''
def check_family(inesk, errors):
    if not inesk.family in [socket.AF_INET, socket.AF_INET6]:
        errors.add_error("wrong family")
    '''
    (CRIU SUPPORTED)
    '''
    pass

def check_type(inetsk, errors):
    if not inetsk.type in [socket.SOCK_STREAM, socket.SOCK_DGRAM]:
        errors.add_error("wrong type")
    '''
    (CRIU SUPPORTED)
    '''
    pass

# @extra_check
# def check_proto_parsing_etc(inetsk, errors):
#     etc_proto = "/etc/protocols"
#     with open(etc_proto, "r") as proto:
#         for line in proto:
#             print(line)

def check_proto(inetsk, errors):
    '''
    IS CRIU is ONLY SUPPORTED TCP ?
    '''
    pass

def check_state(inetsk, errors):
    '''
    TCP_LISTEN:
    TCP_ESTABLISHED:
    TCP_CLOSE:
    '''
    pass

def check_flags(inetsk, errors):
    if not common_checks.check_open_flags(inetsk.flags):
        errors.add_error("wrong flags") 
    pass

def check_backlog(inetsk, errors):
    if not common_checks.check_is_int(inetsk.backlog) or common_checks.check_is_negative(inetsk.backlog):
        errors.add_error("wrong backlog")

def check_ip_addr(family, addr, errors):
    try:
        socket.inet_pton(family, addr)
    except AttributeError:  # no inet_pton here, sorry
        try:
            socket.inet_aton(address)
        except socket.error:
            return False
        return address.count('.') == 3
    except socket.error:  # not a valid address
        return False

    return True


def check_addr(addr, inetsk, errors):
    '''
    WTF - again - wrong parsing addr with reader
    if inetsk.family == AF_INET or inetsk.v6only == False:
        check_ip_addr(AF_INET, addr, errors)
    if inetsk.family == AF_INET6:
        check_ip_addr(AF_INET6, addr, errors)
    '''
    pass

def check_src_addr(intesk, errors):
    for addr in intesk.src_addr:
        check_addr(addr, intesk, errors)
        
def check_dst_addr(intesk, errors):
    for addr in intesk.src_addr:
        check_addr(addr, intesk, errors)

def check_fown(intesk, errors):
    pass

def check_opts(intesk, errors):
    pass

@extra_check
def validate_one_inetsk(inetsk, errors):
    errors.add_context("validating inesk")
    check_id(inetsk, errors)
    check_ino(inetsk, errors)
    check_family(inetsk, errors)
    check_type(inetsk, errors)
    check_proto(inetsk, errors)
    check_state(inetsk, errors)
    check_flags(inetsk, errors)
    check_backlog(inetsk, errors)
    check_src_addr(inetsk, errors)
    check_dst_addr(inetsk, errors)
    check_fown(inetsk, errors)
    check_opts(inetsk, errors)
    errors.pop_context()
            
@extra_check
def validate_inetsk_list(inetsk_list, errors):
    errors.add_context("validating inetsk_list")
    for inetsk in inetsk_list:
        validate_one_inetsk(inetsk, errors)
    errors.pop_context()
    
if __name__ == '__main__':
    dir_path = sys.argv[1]
    enable_extra()
    errors = ErrorList()
    reader = CriuDumpReader(dir_path)
    inesk_list = InetskList(reader, errors)
    validate_inetsk_list(inesk_list, errors)
    errors.show_errors()