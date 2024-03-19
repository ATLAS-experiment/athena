#!/bin/sh
# Emacs, this is mostly -*-Python-*-
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# athena.py is born as shell script to preload some optional libraries.
#
"""date"

# defaults
export USETCMALLOC=0
export USEIMF=0
export USEEXCTRACE=0
otherargs=()
# but use tcmalloc by default if TCMALLOCDIR is defined
if [ -n "$TCMALLOCDIR" ]; then
    export USETCMALLOC=1
fi

# parse LD_PRELOAD related command line arguments
for a in "$@"
do
    case "$a" in
        --leak-check*)   USETCMALLOC=0;;
        --delete-check*) USETCMALLOC=0;;
        --stdcmalloc)    USETCMALLOC=0;;
        --tcmalloc)      USETCMALLOC=1;;
        --stdcmath)      USEIMF=0;;
        --imf)           USEIMF=1;;
        --exctrace)      USEEXCTRACE=1;;
        --preloadlib*)     export ATHENA_ADD_PRELOAD=${a#*=};;
        --drop-and-reload) ATHENA_DROP_RELOAD=1;;
        *)               otherargs+=("$a");;
    esac
done


# Do the actual preloading via LD_PRELOAD and save the original value
export LD_PRELOAD_ORIG=${LD_PRELOAD}
source `which athena_preload.sh `

# Now resurrect ourselves as python script
python_path=`which python`
"exec" "$python_path" "-tt" "$0" "$@";

"""

# File: athena.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)
# "
# This script allows you to run Athena from python.
#
# Debugging is supported with the '-d' option (hook debugger after running
# all user scripts, and just before calling initialize) and the --debug
# option (requires "conf", "init", or "exec" and will hook just before that
# stage). The hook will give you the gdb prompt, from where you can set
# break points, load additional shared libraries, or drop into interactive
# athena mode (if -i specified on the cli). Alternatively, you can start
# with gdb, like so:
#
#  $ gdb python
#  (gdb) run `which athena.py` [options] [<file1>.py [<file2>.py ...
#
# Usage of valgrind is supported, but it requires full paths and explicit
# arguments in its run, like so:
#
#  $ valgrind `which python` `which athena.py` [options] [<file1>.py ...
#
# or, alternatively (valgrind 3.2.0 and up):
#
#  $ valgrind --trace-children=yes `which athena.py` [options] [<file1>.py ...
#
# Note that any error messages/leaks that valgrind reports on python can be
# ignored, as valgrind is wrong (see the file Misc/README.valgrind in the
# python installation).
#

__author__  = 'Wim Lavrijsen (WLavrijsen@lbl.gov)'
__doc__     = 'For details about athena.py, run "less `which athena.py`"'

import sys, os

### parse the command line arguments -----------------------------------------
import AthenaCommon.AthOptionsParser as aop
aop.enable_athenaCLI()
opts = aop.parse(legacy_args=True)

### inspect first script or pickle to determine legacy/CA mode
if opts.scripts:
   from AthenaCommon.Utils.unixtools import FindFile
   path_list = ['./'] + os.environ.get('PYTHONPATH', '').split(os.pathsep)
   file_path = FindFile( os.path.expanduser( os.path.expandvars(opts.scripts[0]) ),
                         path_list, os.R_OK )

   if file_path is not None:
      with open(file_path) as f:
         if f.readline().startswith('#!'):  # shebang means CA mode
            opts.CA = True

      if opts.CA:
         sys.argv.remove(opts.scripts[0])  # drop script path from args
         opts.scripts[0] = file_path       # replace with resolved path

elif opts.fromdb:
   import pickle
   with open(opts.fromdb, 'rb') as f:
      try:
         acc = pickle.load(f)
         opts.CA = not isinstance(acc, dict)  # legacy pkl is a dict
         if not opts.CA:
            del acc   # legacy pkl is loaded in Execution.py
      except ModuleNotFoundError:
         pass   # in case ComponentAccumulator class is not available in release

### remove preload libs for proper execution of child-processes --------------
if 'LD_PRELOAD_ORIG' in os.environ:
   os.environ['LD_PRELOAD'] = os.getenv('LD_PRELOAD_ORIG')
   os.unsetenv('LD_PRELOAD_ORIG')

### debugging setup
from AthenaCommon.Debugging import DbgStage
DbgStage.value = opts.debug

### python interpreter configuration -----------------------------------------
if not os.getcwd() in sys.path:
   sys.path = [ os.getcwd() ] + sys.path

if '' not in sys.path:
   sys.path = [ '' ] + sys.path


## rename ourselfs to athena, both the prompt and the process (for top & ps)
sys.ps1 = 'athena> '
sys.ps2 = '.   ... '

try:
   import ctypes
   from ctypes.util import find_library as ctypes_find_library
   libc = ctypes.cdll.LoadLibrary( ctypes_find_library('c') )
   libc.prctl( 15, b'athena.py', 0, 0, 0 )
except Exception:
   pass            # don't worry about it failing ...


## interface setup as appropriate
if not (opts.interactive or opts.debug):
 # in batch there is no need for stdin
   if sys.stdin and os.isatty( sys.stdin.fileno() ):
      os.close( sys.stdin.fileno() )
else:
   # Make sure ROOT gets initialized early, so that it shuts down last.
   # Otherwise, ROOT can get shut down before Gaudi, leading to crashes
   # when Athena components dereference ROOT objects that have been deleted.
   import ROOT  # noqa: F401

### CA mode
if opts.CA:
   from AthenaCommon import ExitCodes
   exitcode = 0
   try:
      if opts.scripts:  # CA script
         if not opts.tracelevel:
            import runpy
            runpy.run_path( opts.scripts[0], run_name='__main__' )
         else:
            from AthenaCommon.Debugging import traceExecution
            traceExecution( opts.scripts[0], opts.tracelevel )

      elif opts.fromdb:  # pickle
         from AthenaCommon.AthOptionsParser import configureCAfromArgs
         configureCAfromArgs( acc, opts )
         sys.exit(acc.run(opts.evtMax).isFailure())

   except SystemExit as e:
      # Failure in ComponentAccumulator.run() is very likely an algorithm error
      exitcode = ExitCodes.EXE_ALG_FAILURE if e.code==1 else e.code

   # FIXME: change the print to log (requires ref updates)
   #from AthenaCommon import Logging
   #Logging.log.info( 'leaving with code %d: "%s"',
   #                  e.code, ExitCodes.what(e.code) )
   print( 'leaving with code %d: "%s"' % (exitcode, ExitCodes.what(exitcode)) )
   sys.exit( exitcode )

### Legacy mode
else:
   # logging and messages
   from AthenaCommon.Logging import logging, log
   _msg = log

   # test and set log level
   try:
      _msg.setLevel (getattr(logging, opts.loglevel))
   except Exception:
      aop._help_and_exit()

   # start profiler, if requested
   if opts.profile_python:
      import cProfile
      # profiler is created and controlled programmatically b/c a CLI profiling of
      # athena.py doesn't work (globals are lost from include() execfile() calls),
      # and because this allows easy excluding of the (all C++) Gaudi run
      cProfile._athena_python_profiler = cProfile.Profile()
      cProfile._athena_python_profiler.enable()

   # Fill athena job properties
   aop.fill_athenaCommonFlags(opts)

   # file inclusion and tracing
   from AthenaCommon.Include import include
   include.setShowIncludes(opts.showincludes)

   # pre-execution step
   include( "AthenaCommon/Preparation.py" )

   # execution of user script and drop into batch or interactive mode ---------
   include( "AthenaCommon/Execution.py" )
