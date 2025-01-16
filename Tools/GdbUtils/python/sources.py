# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: GdbUtils/python/sources.py
# Created: A while ago, sss
# Purpose: Try to automatically set source file paths for an ATLAS build.
#

import os
import gdb

# Set source path for ROOT.
# rootsys = os.environ.get ('ROOTSYS')
# if rootsys:
#     rootsys = os.path.dirname (rootsys)
#     rootsys = os.path.dirname (rootsys)
#     rootsys = os.path.join (rootsys, 'src', 'root')
#     if os.path.exists (rootsys):
#         gdb.execute ("dir %s" % rootsys)


# Hacks to set other source paths properly.

# def _handle_atlas_buildarea (fname):
#     # Work around realpath bug in py 2.3.
#     if os.path.islink(fname):
#         fname = os.path.join (os.path.dirname (fname),
#                               os.readlink (fname))
#     fname = os.path.realpath (fname)
#     if os.path.exists (fname):
#         fname = os.path.dirname (fname)
#         pkgdir = os.path.dirname (fname)
#         pkg = os.path.basename (pkgdir)
#         gdb.execute ("dir %s" % os.path.join (pkgdir, 'src'))
#         gdb.execute ("dir %s" % os.path.join (pkgdir, pkg))
#     return

# _lcg_done = ['ROOT'] # root's handled separately.
# def _handle_lcgarea (fname):
#     fname = os.path.realpath (fname)
#     if os.path.exists (fname):
#         fname = os.path.dirname (fname)  # .../lib
#         fname = os.path.dirname (fname)  # .../arch
#         fname = os.path.dirname (fname)  # .../PRODUCT_VERSION

#         product = os.path.dirname (fname) # .../PRODUCT
#         product = os.path.basename (fname) # PRODUCT
#         if product in _lcg_done: return
#         srcdir = os.path.join (fname, 'src')
#         if not os.path.exists (srcdir): return
#         paths = os.listdir (srcdir)
#         for p in paths:
#             dir = os.path.join (srcdir, p, 'src')
#             if os.path.exists (dir):
#                 gdb.execute ("dir %s" % dir)
#             dir = os.path.join (srcdir, p, p)
#             if os.path.exists (dir):
#                 gdb.execute ("dir %s" % dir)
#     return


_handled_dirs=set()
def handle_comp_dir (fname, dirname):
    if dirname in _handled_dirs:
        return
    _handled_dirs.add (dirname)
    if not dirname.startswith ('/build'): return

    pos = dirname.rfind ('/build/')
    if pos < 0: return
    pos += 7
    pos = dirname.find ('/', pos)
    if pos < 0: return
    pack = dirname[pos+1:]

    pos = fname.find ('/InstallArea/')
    if pos < 0: return
    pos += 13
    pos = fname.find ('/', pos)
    if pos < 0: return
    srcroot = fname[:pos] + '/src'
    pdir = srcroot + '/' + pack
    for (dirpath, dirnames, filenames) in os.walk (pdir):
        print ('dir ' + dirpath)
        gdb.execute ('dir ' + dirpath)
    return

_handled=set()
def handle_atlas_buildarea (fname):
    if fname in _handled:
        return
    import subprocess
    files = set()
    out = subprocess.getoutput ('objdump --dwarf=info %s | grep DW_AT_comp_dir' % fname)
    for l in out.split('\n'):
        pos = l.find('/')
        if pos >= 0:
            files.add (l[pos:].strip())
    for f in files:
        handle_comp_dir (fname, f)
    _handled.add(fname)
    return


def _objfile_hook (ev):
    o = ev.new_objfile
    if o.filename.startswith ('/cvmfs'):
        handle_atlas_buildarea (o.filename)
    return

gdb.events.new_objfile.connect (_objfile_hook)
