# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/sheap.py
# Created: A while ago, sss
# Purpose: Dump out malloc heap regions.
#

import gdb

def sheap_read_word (inf, addr):
    mv = inf.read_memory (addr, 8)
    return mv.cast('L')[0]

def sheap_looks_free (inf, addr):
    head = sheap_read_word (inf, addr+8)
    if head > 128*1024*1024 or head < 8: return False
    #print (hex(addr), hex(head))
    head = head & ~1
    if head & 2: return False
    foot = sheap_read_word (inf, addr+head)
    if head != foot: return False
    nexthead = sheap_read_word (inf, addr+head+8)
    if nexthead & 1: return False
    #print ('found', hex(addr), hex(head), hex(foot), hex(nexthead))
    return True

def sheap_scan_2 (inf, addr):
    head = sheap_read_word (inf, addr+8)
    if head > 128*1024*1024 or head < 8:
        return addr+8, False
    head = head & ~1
    if head & 2:
        return addr+8, False
    nexthead = sheap_read_word (inf, addr+head+8)
    if nexthead & 1:
        type = ' USED '
    else:
        if not sheap_looks_free (inf, addr):
            return addr+8, False
        type = ' FREE '
    #print('sheap_scan_2' ,hex(addr), hex(head), hex(nexthead), type)
    s = f'{int(addr):016x}:{type}{int(head):08x}\n'
    gdb.write (s)
    return addr+head, True

def sheap_scan_1 (addr1, addr2, forced):
    inf = gdb.selected_inferior()
    addr1 = (addr1 + 7) & ~7 # Align
    if not forced:
        while addr1 < addr2 and not sheap_looks_free (inf, addr1):
            addr1 += 8
    while addr1 < addr2:
        addr1, ok = sheap_scan_2 (inf, addr1)
        if not ok: break
    return addr1

def sheap_scan (addr1, addr2, forced):
    addr1 -= 8
    while addr1 < addr2:
        addr1 = sheap_scan_1 (addr1, addr2, forced)
        forced = False
    gdb.flush()
    return


def sheap (arg, forced):
    args = arg.split()
    if not args:
        raise gdb.GdbError ('Missing start address')
    start_arg = gdb.parse_and_eval (args[0])
    end_arg = gdb.parse_and_eval (args[1]) if len(args) > 1 else start_arg + 1024
    return sheap_scan (start_arg, end_arg, forced)


class SHeap (gdb.Command):
    """Dump out malloc heap in a range of addresses, showed allocated/free blocks.

    sheap START-ADDR [END-ADDR]
    will dump out free and allocated malloc blocks within the given address
    range.  If not given, END-ADDR is taken to be 1024 bytes after START-ADDR.

    Caveats: It is not possible to reliably identify allocated memory blocks.
             Dumping will start at the first free block found within
             the given range, if any.

             This only works with the GNU libc malloc, not with any
             alternative allocators (tcmalloc, etc).
    """
    def __init__ (self):
        super (SHeap, self).__init__ ("sheap", gdb.COMMAND_DATA)
        return

    def invoke (self, arg, from_tty):
        return sheap (arg, False)


SHeap()
