# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/get_inferior.py
# Created: A while ago, sss
# Purpose: Return the PID of the current inferior.
#


import gdb

def get_inferior():
    """Return the PID of the current inferior, or None."""
    inf = gdb.selected_inferior()
    if inf is None: return None
    return inf.pid

