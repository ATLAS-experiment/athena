# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: GdbUtils/python/init.py
# Created: A while ago, sss
# Purpose: Initialize the ATLAS gdb utilities.
#
# You can load this from the gdb command line (or .gdbinit) with
#  python import GdbUtils
#

import sys
import os
import gdb

# Find the directory in which these scripts are installed and add it to
# the python path.
thisdir = os.path.dirname (sys.modules[__package__].__file__)
sys.path = [thisdir] + sys.path

# Also put this directory the source path, for `source'.
gdb.execute ("dir %s" % thisdir)

# Load commands enabled by default.
import findlib    # noqa: F401
import sources    # noqa: F401
import btload     # noqa: F401
import pahole     # noqa: F401
import offsets    # noqa: F401
import importcmd  # noqa: F401
import sheap      # noqa: F401
