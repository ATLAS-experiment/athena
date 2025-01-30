# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaCommon/python/Debugging.py
## @brief py-module to hold a few tools and utilities to help debugging
##        configurables and/or Athena application
## @author Sebastien Binet <binet@cern.ch>
###############################################################

__doc__ = """py-module to hold a few tools and utilities to help debugging
configurables and/or Athena application.
"""

import io

class DbgStage:
    """Class to hold the stage at which the user asked to hook the debugger
    """
    value          = None
    allowed_values = ( "conf", "init", "exec", "fini" )
    pass # class DbgStage

def hookDebugger():
    """debugging helper, hooks debugger to running interpreter process
       gdb can be overridden via $ATLAS_DEBUGGER
    """
    import os
    debugger = os.environ.get('ATLAS_DEBUGGER', 'gdb')

    pid = os.spawnvp(os.P_NOWAIT,
                     debugger, [debugger, '-q', 'python', str(os.getpid())])

    # give debugger some time to attach to the python process
    import time
    time.sleep( 1 )

    # verify the process' existence (will raise OSError if failed)
    os.waitpid( pid, os.WNOHANG )
    os.kill( pid, 0 )
    return

def hookStrace(out=None):
    """Same than for hookDebugger: spawn strace and attach to the running
    interpreter process
    """
    import os
    if out is None: out = 'athena.strace.log.%i' % os.getpid()
    if isinstance(out, io.IOBase):
        out = out.name
    elif not isinstance(out,str):
        raise TypeError('argument 0 needs to be either a file or a filename')
    
    pid = os.spawnvp(os.P_NOWAIT,
                     'strace', ['strace',
                                '-f', # follow children
                                '-p', str(os.getpid()),
                                '-o', out])

    # give strace some time to attach to the python process
    import time
    time.sleep( 1 )

    # verify the process' existence (will raise OSError if failed)
    os.waitpid( pid, os.WNOHANG )
    os.kill( pid, 0 )
    return


def allowPtrace():
    """On kernels with Yama enabled, ptrace may not work by default on processes
which are not decendants of the tracing process.  Among other things, that
causes the way we attach the debugger to fail.  However, we can disable this
on a per-process basis.  Do that here.

See https://www.kernel.org/doc/Documentation/security/Yama.txt and prctl(2).
"""

    # First test to see if ptrace restrictions are enabled.
    import os
    # Return if this kernel does not support ptrace restrictions.
    if not os.path.exists ('/proc/sys/kernel/yama/ptrace_scope'): return

    # Return if ptrace restrictions are disabled.
    with open('/proc/sys/kernel/yama/ptrace_scope') as f:
        if f.readline().strip() == '0':
            return

    # Use prctl to try to enable ptrace.
    from ctypes import CDLL
    libc = CDLL("libc.so.6")
    # Args are PTRACE_SET_PTRACER (4HYama) and
    # PR_SET_PTRACER_ANY ((unsigned long)-1).
    libc.prctl (0x59616d61, 0xffffffffffffffff)

    return


def dumpPythonProfile(filename):
    """Save python profile data of the default athena profiler instance
    into filename (.txt or .pkl format).
    """
    from AthenaCommon.Logging import log
    import cProfile
    import pstats

    profiler = cProfile._athena_python_profiler
    profiler.disable()
    if filename.endswith(".txt"):
        stats = pstats.Stats(profiler, stream=open(filename, 'w'))
        stats.strip_dirs().sort_stats("time").print_stats()
        log.info("Python profile summary stored in %s", filename)
    else:
        profiler.dump_stats(filename)
        log.info("Python profile stored in %s", filename)


def traceExecution(script, level):
    """Run script with the given trace level"""
    import sys
    import trace
    ignore_dirs = []
    ignore_mods = []

    # exclude system and root libraries
    if level > 0:
        ignore_dirs = [x for x in sys.path if x.find("ROOT")!=-1]
        ignore_dirs.append(sys.prefix)
    if level > 1:
        ignore_mods += ['_db', '__init__', '_configurables', 'GaudiHandles', 'six']
    if level > 2:
        ignore_mods += [
            # Legacy stuff we pull in b/c PythonAlgorithms and such still derive from old-style configurables
            'Configurable','Configurables','PropertyProxy','DataHandle','ConfigurableDb',
            'ConfigurableMeta', 'CfgMgr',
            # Internals of the ComponentAccumulator
            'ComponentAccumulator','Logging','AthConfigFlags','AllConfigFlags','Deduplication',
            'semantics','AccumulatorCache','DebuggingContext','AtlasSemantics','ComponentFactory',
            'LegacySupport','ItemListSemantics'
        ]

    # taken from cpython/Lib/trace.py
    tracer = trace.Trace(ignoredirs=ignore_dirs, ignoremods=ignore_mods)

    with io.open_code(script) as f:
        code = compile(f.read(), script, 'exec')
    tracer.run(code)
