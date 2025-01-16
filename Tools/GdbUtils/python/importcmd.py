# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/findlib.py
# Created: A while ago, sss
# Purpose: Look up a shared library by address.
#


import gdb

class ImportCmd (gdb.Command):
    """Shorthand for python import.

    import MOD will import the python module MOD.
    """

    def __init__ (self):
        super (ImportCmd, self).__init__ ("import", gdb.COMMAND_FILES)
        return

    def invoke (self, arg, from_tty):
        import importlib
        importlib.import_module (arg)
        return

ImportCmd()
