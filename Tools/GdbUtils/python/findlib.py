# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/findlib.py
# Created: A while ago, sss
# Purpose: Look up a shared library by address.
#


import gdb
import os
from get_inferior import get_inferior


def findlib (addr, quiet = False):
  inf = get_inferior()
  if inf is None:
    print ('No inferior.')
    return
  maps = open ('/proc/%d/maps' % inf)
  for l in maps.readlines():
    if l and l[-1] == '\n':
      l = l[:-1]
    ll = l.split()
    (lo,hi) = ll[0].split('-')
    lo = int(lo, 16)
    hi = int(hi, 16)
    if lo <= addr < hi:
      if len(ll) >= 6 and ll[5] != '[heap]':
        lib = ll[5]
        lib = os.path.basename (lib)
        ipos = lib.find('.so')
        if ipos > 0:
          lib = lib[:ipos+3]
        if quiet:
          gdb.execute ("shared " + os.path.basename (lib), to_string = True)
        else:
          print (lib)
          gdb.execute ("shared " + os.path.basename (lib))
  return


class FindLib (gdb.Command):
  """Look up a shared library by address.

  findlib ADDR will look up the shared library containing ADDR, if any.
  If found, then its symbols will be loaded.
  """

  def __init__ (self):
    super (FindLib, self).__init__ ("findlib", gdb.COMMAND_FILES)

  def invoke (self, arg, from_tty):
    addr = int(arg.split()[0], 0)
    return findlib (addr)


FindLib()

