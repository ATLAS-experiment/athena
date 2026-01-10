### preparation code before touching any user scripts ------------------------
import AthenaCommon.Include as AthCIncMod
if opts.showincludes:
   AthCIncMod.marker = ' -#-'      # distinguish bootstrap from other jo-code

### setup interactive prompt
if opts.interactive:
   from AthenaCommon.Interactive import configureInteractivePrompt
   configureInteractivePrompt()
   del configureInteractivePrompt

### debugging helper, hooks debugger to running interpreter process ----------
from AthenaCommon.Debugging import hookDebugger, allowPtrace
allowPtrace()

### athena/gaudi -------------------------------------------------------------
from AthenaCommon.Configurable import *

### some useful constants ----------------------------------------------------
from AthenaCommon.Constants import *
# namespace these constants: Lvl.INFO
# so we can write e.g: Units.GeV
import AthenaCommon.Constants     as Lvl
import AthenaCommon.SystemOfUnits as Units

### Athena configuration -----------------------------------------------------
from AthenaCommon import CfgMgr
from AthenaCommon.AppMgr import ( athAlgSeq, theApp, ToolSvc, theAuditorSvc,
                                  ServiceMgr, ServiceMgr as svcMgr )
from AthenaCommon.Logging import log

# load all entries so far into the workspace of include()

if opts.interactive:                                 # i.e. interactive
   theApp.EventLoop = "PyAthenaEventLoopMgr"         # from AthenaServices

## create the application manager and start in a non-initialised state
theApp.setOutputLevel( globals()[opts.loglevel] )
theApp._opts = opts                                     # FIXME

## further job messaging configuration
if not "POOL_OUTMSG_LEVEL" in os.environ:
   os.environ[ "POOL_OUTMSG_LEVEL" ] = str(globals()[opts.loglevel])

## basic job configuration
import AthenaCommon.AtlasUnixStandardJob

from PyUtils.Helpers import ROOTSetup
ROOTSetup(batch=not opts.interactive)
# Make sure that batch mode gets set properly.
# ROOTSetup will set things up so that whenever we do an `import ROOT',
# then gROOT.SetBatch will be called properly.
# However, if someone instead writes `from ROOT import gSystem'
# (or anything other than gROOT), then the ROOT python update thread
# will be started before ROOTSetup calls SetBatch.
# (This was then problematic for some MP jobs which used python
# algorithms.)
# So include the import here explictly to ensure that batch mode gets set.
# The following call to SetBatch is strictly redundant, but included
# for clarity.
import ROOT
ROOT.gROOT.SetBatch(not opts.interactive)


## now import the top-level module which eases interactive work and/or
## python-based components: a pure python module
## we have to put it there as some jobOptions might steal the event loop...
## ie: do theApp.initialize();theApp.nextEvent(...) directly in the jobO.

## user level configuration
from AthenaCommon.Include import IncludeError
try:
   include( "$HOME/.athenarc" )
except IncludeError:
   pass

### max out resource limits --------------------------------------------------
from AthenaCommon.ResourceLimits import SetMaxLimits
SetMaxLimits()
del SetMaxLimits
del sys.modules[ 'AthenaCommon.ResourceLimits' ]

### prettification for interactive use
if opts.interactive:
   import atexit

 # finalize on exit (^D)
   atexit.register( theApp.exit )

   del atexit

if opts.command:
   _msg.info( 'executing CLI (-c) command: "%s"' % opts.command )
   exec (opts.command)

if DbgStage.value == "conf":
   hookDebugger()

if opts.showincludes:
   AthCIncMod.marker = AthCIncMod.__marker__      # reset
del AthCIncMod

## basic job configuration for Hive and AthenaMP

from AthenaCommon.ConcurrencyFlags import jobproperties as jps
if opts.nprocs and (opts.nprocs >= 1 or opts.nprocs==-1):
   # MPI and AthenaMP don't mix
   if opts.mpi:
      _msg.error("Cannot run AthenaMP with MPI")
      sys.exit()
   jps.ConcurrencyFlags.NumProcs = opts.nprocs
   _msg.info ("configuring AthenaMP with [%s] sub-workers", 
              jps.ConcurrencyFlags.NumProcs())

   if (opts.debug_worker is True) :
      jps.ConcurrencyFlags.DebugWorkers = True
      _msg.info ("   Workers will pause after fork until SIGUSR1 signal received")

if (opts.threads or opts.concurrent_events) :

   if (opts.threads is None and opts.concurrent_events is not None) :
      ## num threads = num concurrent evts
      jps.ConcurrencyFlags.NumThreads = opts.concurrent_events
      jps.ConcurrencyFlags.NumConcurrentEvents = opts.concurrent_events
   elif (opts.threads is not None and opts.concurrent_events is None) :
      ## num concurrent evts = num threads
      jps.ConcurrencyFlags.NumThreads = opts.threads
      jps.ConcurrencyFlags.NumConcurrentEvents = opts.threads
   else :
      ## both concurrent evts and threads set individually
      jps.ConcurrencyFlags.NumThreads = opts.threads
      jps.ConcurrencyFlags.NumConcurrentEvents = opts.concurrent_events
         

   if (jps.ConcurrencyFlags.NumProcs() > 0) :
      _msg.info ("configuring hybrid AthenaMP/AthenaMT with [%s] concurrent threads and [%s] concurrent events per AthenaMP worker", jps.ConcurrencyFlags.NumThreads, jps.ConcurrencyFlags.NumConcurrentEvents)
   elif (jps.ConcurrencyFlags.NumProcs() == 0) :
      _msg.info ("configuring AthenaHive with [%s] concurrent threads and [%s] concurrent events", jps.ConcurrencyFlags.NumThreads(), jps.ConcurrencyFlags.NumConcurrentEvents())
   else:
      # we should never get here
      _msg.error ("ConcurrencyFlags.NumProcs() cannot == -1 !!")
      sys.exit()

   import AthenaCommon.AtlasThreadedJob
