# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/stl.py
# Created: A while ago, sss
# Purpose: Enable pretty-printing of STL types.
#

import glob
import os
import sys
import subprocess

gccbin = subprocess.getoutput ('which gcc')
gccdir = os.path.dirname(os.path.dirname(gccbin))
gccpydir = glob.glob(gccdir + '/share/*/python')[0]
sys.path.append (gccpydir)

from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers(None)
