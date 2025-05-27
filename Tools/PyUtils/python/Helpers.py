# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# @author: Sebastien Binet <binet@cern.ch>
# @date:   March 2007
#
#

import re
import os
import sys
from functools import cache
from tempfile import NamedTemporaryFile
# !!!
# Do not add any Athena dependencies at the module-level as some of the code here
# is used during the build (see athena!62739).
# !!!

def ROOT6Setup(batch=False):
   """
   At this point this function just makes sure the ROOT batch mode is set
   correctly when someone imports ROOT
   """
   from AthenaCommon.Logging import log
   log.info('executing ROOT6Setup')
   import builtins as builtin_mod
   oldimporthook = builtin_mod.__import__
   batch_mode = bool(batch)

   def root6_importhook(name, globals={}, locals={}, fromlist=[], level=0):
       nonlocal batch_mode
       isroot = False
       bm = batch_mode
       if name=='ROOT' or (name[0:4]=='ROOT' and name!='ROOT.pythonization'):
          isroot = True
          batch_mode = None  # only set it on first ROOT import

       m = oldimporthook(name, globals, locals, fromlist, level)

       if m and isroot:
          log.debug('Python import module=%s, fromlist=%s', name, fromlist)
          if bm is not None:
             log.debug('Setting ROOT batch mode to %s', bm)
             m.gROOT.SetBatch(bm)

       return m

   builtin_mod.__import__ = root6_importhook


class ShutUp(object):
    """
    A little helper class to keep ROOT silent...
    """
    DefaultFilter = [
        re.compile("Warning in <TClass::TClass>: no dictionary for class."),
        re.compile("Warning in <TEnvRec::ChangeValue>: duplicate entry."),
        re.compile("Error in <TStreamerInfo::BuildOld>:."),
        ]
    def __init__(self, filters = DefaultFilter):
        self._dummy = False # if dummy, we don't really shut-up ROOT...
        if os.environ.get('PYUTILS_SHUTUP_DEBUG', '0') == '1':
            self._dummy = True
            
        self.save_err  = open( '/dev/null', 'w' )
        self.save_out  = open( '/dev/null', 'w' )
        try:
            self.quiet_err = NamedTemporaryFile( suffix = ".msg.log" )
            self.quiet_out = NamedTemporaryFile( suffix = ".msg.log" )
        except OSError:
            # load problem ?
            # retry a bit later... (in 2 seconds)
            import time
            time.sleep(2) #
            try:
                self.quiet_err = NamedTemporaryFile( suffix = ".msg.log" )
                self.quiet_out = NamedTemporaryFile( suffix = ".msg.log" )
            except OSError:
                # then (implicitly) fallback on sys.stderr
                self._dummy = True

        self.filters = filters

        if not self._dummy:
            os.dup2( sys.stderr.fileno(), self.save_err.fileno() )
            os.dup2( sys.stdout.fileno(), self.save_out.fileno() )
        return

    def __del__ (self):
       self.save_err.close()
       self.save_out.close()
       return
    
    def mute(self):
        if not self._dummy:
            os.dup2( self.quiet_err.fileno(), sys.stderr.fileno() )
            os.dup2( self.quiet_out.fileno(), sys.stdout.fileno() )
        return
    
    def unMute(self):
        if not self._dummy:
            os.dup2( self.save_err.fileno(), sys.stderr.fileno() )
            os.dup2( self.save_out.fileno(), sys.stdout.fileno() )
            self.__filterRootMessages(self.quiet_err)
            self.quiet_err.seek(0)
            self.__filterRootMessages(self.quiet_out)
            self.quiet_out.seek(0)
        return

    def __filterRootMessages(self, fd):
        fd.seek(0)
        for l in fd.readlines():
            l = l.decode()
            printOut = True
            for filter in self.filters:
                if re.match(filter, l):
                    printOut = False
            if printOut:
                print ("PyRoot:",l.replace("\n",""))
            pass
        return

    # context-manager protocol
    def __enter__(self):
        return self.mute()
    def __exit__(self,exc_type, exc_val, exc_tb):
        return self.unMute()


@cache
def release_metadata():
   """Returns information about the current release based on ReleaseData"""

   import configparser
   d = {
      'project name': '?',
      'release': '?',
      'base release': '?',
      'nightly release': '?',
      'nightly name': '?',
      'date': '?',
      'platform': '?',
      }

   for cmake_path in os.environ['CMAKE_PREFIX_PATH'].split(os.pathsep):
      release_data = os.path.join(cmake_path, 'ReleaseData')
      if os.path.exists(release_data):
         d1=d
         cfg = configparser.ConfigParser()
         try:
            cfg.read( release_data )
            if cfg.has_section( 'release_metadata' ):
               d1.update( dict( cfg.items( 'release_metadata' ) ) )
               d1['platform'] = os.getenv( '%s_PLATFORM' % d1['project name'],
                                           '?' )
               release = d1['release'].split('.')
               base_release = d1['base release'].split('.')
               if len(release)>=3 or len(base_release)>=3:
                  return d1
         except Exception:
            pass
   return d
