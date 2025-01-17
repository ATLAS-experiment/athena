# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/findlib.py
# Created: A while ago, sss
# Purpose: When printing a pointer, try to look up its dynamic type.
#

import gdb
from findlib import findlib


class VPointerPrinter:
    def __init__ (self, val):
        self.val = val


    def vtstring1 (self, vt):
        olddem = gdb.parameter('print asm-demangle')
        out = ''
        try:
            try:
                gdb.execute ('set print asm-demangle on')
                out = gdb.execute ('x/x 0x%x' % int(vt), to_string = True)
            except: # noqa: E722 B001
                # Suppress errors
                pass
        finally:
            if not olddem:
                gdb.execute ('set print asm-demangle off')
        i = out.find ('<vtable for ')
        if i >= 0:
            i += 12
            j = out.rfind ('>')
            if j >= 0:
                jj = out.rfind ('+', i, j)
                if jj >= 0:
                    j = jj
                return '<' + out[i:j] + '>'

        return ''


    def vtstring (self):
        if self.val == 0: return ''
        vpptype = gdb.lookup_type('void').pointer().pointer()
        vt = self.val.cast(vpptype).dereference()
        out = self.vtstring1 (vt)
        if not out:
            # FIXME: Cache somehow to prevent many calls to findlib.
            findlib (int(vt), True)
            out = self.vtstring1 (vt)
        return out


    def to_string (self):
        return "(%s) 0x%x%s" % (self.val.type, int(self.val), self.vtstring())


#_vpointer_types = ['IAddressProvider',
#                   'pool::IPersistencySvc']
#_vpointer_dict = dict([('class ' + v,1) for v in _vpointer_types])
def lookup_vpointer_function (val):
    "Look-up and return a pretty-printer that can print val."

    # Get the type.
    type = val.type

    # If it points to a reference, get the reference.
    if type.code == gdb.TYPE_CODE_REF:
        type = type.target ()

    # Get the unqualified type, stripped of typedefs.
    type = type.unqualified ().strip_typedefs ()

    if type.code != gdb.TYPE_CODE_PTR:
        return

    #if str(type.target()).startswith ('class '):
    if type.target().code == gdb.TYPE_CODE_STRUCT:
        return VPointerPrinter(val)

    # Cannot find a pretty printer.  Return None.
    return None


gdb.pretty_printers.insert (0, lookup_vpointer_function)
