# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/btload.py
# Created: A while ago, sss
# Purpose: Load shared libraries for the current backtrace.
#


import gdb
from findlib import findlib


def btload (limit = 100):
    retry = True
    last_iframe = -1
    while retry:
        frame = gdb.newest_frame()
        iframe = 0
        retry = False
        while frame and frame.is_valid() and iframe < limit:
            if frame.name() is None:
                findlib (frame.pc())
                retry = True
                break
            if frame.name() == 'ApplicationMgr::executeRun':
                break
            if frame.name() == '_Py_UnixMain':
                break
            frame = frame.older()
            iframe = iframe + 1
        if iframe <= last_iframe: break
        last_iframe = iframe
    return


class BTLoad (gdb.Command):
    """Load shared libraries from the current backtrace."""

    def __init__ (self):
        super (BTLoad, self).__init__ ("btload", gdb.COMMAND_FILES)
        return

    def invoke (self, arg, from_tty):
        limit = 100
        args = arg.split()
        if len(args) > 0:
            limit = int(args[0])
        btload (limit)
        return

BTLoad()
