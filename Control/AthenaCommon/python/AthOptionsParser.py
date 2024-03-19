# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file AthenaCommon.AthOptionsParser
# @purpose the central module to parse command line options of athena.py

import argparse
import os
import sys

## argparse 'action' helpers:

class JobOptAction(argparse.Action):
    """Check filename extension and fill relevant options"""
    def __call__(self, parser, args, values, option_string=None):
        scripts = [f  for f in values if f[-3:] == '.py']
        pkls    = [f  for f in values if f[-4:] == '.pkl']

        if (scripts and pkls) or len(pkls)>1:
            raise ValueError('Only job options or one pickle file is allowed')

        setattr(args, self.dest, scripts)
        setattr(args, 'fromdb', pkls[0] if pkls else None)


class MemCheckAction(argparse.Action):
    """Enable Hepheastus"""
    def __call__(self, parser, args, values, option_string=None):

        setattr(args, self.dest,
                [] if values=='all' else [values])

        # early import is needed for proper offloading later
        import Hephaestus.MemoryTracker as memtrack  # noqa: F401

        if option_string=='--delete-check':
            setattr(args, 'memchk_mode', 'delete-check')
            import Hephaestus.DeleteChecker          # noqa: F401
        else:
            setattr(args, 'memchk_mode', 'leak-check')


class AthHelp(argparse.Action):
   """Custom help to hide/show expert groups"""
   def __call__(self, parser, namespace, values, option_string=None):

      for g in parser.expert_groups:
         for a in g._group_actions:
            if values!='all':
               a.help = argparse.SUPPRESS

      parser.print_help()
      if values!='all':
         print('\nUse --help=all to show all (expert) options')
      sys.exit(0)


def get_version():
    """Version string"""
    from PyUtils.Helpers import release_metadata
    return ('[%(project name)s-%(release)s] [%(platform)s] '
            '[%(nightly name)s/%(nightly release)s] -- built on [%(date)s]' % release_metadata())


def set_environment(opts):
    """Set required envirnoment variables based on command line"""

    # user decision about TDAQ ERS signal handlers
    if opts.enable_ers_hdlr == 'y':
        os.unsetenv('TDAQ_ERS_NO_SIGNAL_HANDLERS')
    else:
        os.environ['TDAQ_ERS_NO_SIGNAL_HANDLERS'] = '1'

    os.environ['LIBC_FATAL_STDERR_'] = '1'  # ATEAM-241


def check_tcmalloc(opts):
    libname = 'libtcmalloc'  # also covers libtcmalloc_minimal.so
    # Warn if...
    if ( libname not in os.getenv('LD_PRELOAD','') and  # tcmalloc not loaded
         os.getenv('USETCMALLOC') in ('1', None) and    # but requested (or default)
         opts.do_leak_chk is None ):                    # and not disabled by leak checker

        print ('*******************************************************************************')
        print ('WARNING: option --tcmalloc used or implied, but libtcmalloc.so not loaded.')
        print ('         This is probably because you\'re using athena.py in a non standard way')
        print ('         such as "python athena.py ..." or "nohup athena.py"')
        print ('         If you wish to use tcmalloc, you will have to manually LD_PRELOAD it')
        print ('*******************************************************************************')
        print ('')


def fill_athenaCommonFlags(opts):
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

    if opts.filesInput is not None:
        import glob
        files = []
        for fe in opts.filesInput.split(","):
            found = glob.glob(fe)
            # if not found, add string directly
            files += found if found else [fe]

        athenaCommonFlags.FilesInput.set_Value_and_Lock(files)

    if opts.evtMax is not None:
        athenaCommonFlags.EvtMax.set_Value_and_Lock(opts.evtMax)

    if opts.skipEvents is not None:
        athenaCommonFlags.SkipEvents.set_Value_and_Lock(opts.skipEvents)


def getArgumentParser():

    parser = argparse.ArgumentParser(prog='athena.py', formatter_class=
                                     lambda prog : argparse.HelpFormatter(
                                         prog, max_help_position=40, width=100),
                                     usage = '%(prog)s [OPTION]... [scripts ...]',
                                     add_help=False)

    parser.expert_groups = []   # List of expert option groups

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('Main options')

    g.add_argument('scripts', nargs='*', action=JobOptAction,
                   help='scripts or pickle file to run')

    g.add_argument('--filesInput', metavar='FILES',
                   help='set FilesInput property (comma-separated list with wildcards)')

    g.add_argument('--evtMax', metavar='N', type=int,
                   help='max number of events to process')

    g.add_argument('--skipEvents', metavar='N', type=int,
                   help='number of events to skip')

    g.add_argument('-r', '--repeat-evts', metavar='N', type=int, dest='nbr_repeat_evts',
                   help='number of times to repeat each event from a given input file')

    g.add_argument('-c', '--command', metavar='CMD',
                   help='one-liner, runs before any scripts')

    g.add_argument('-l', '--loglevel', metavar='LVL', type=str.upper, default='INFO',
                   choices=['ALL', 'VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR', 'FATAL'],
                   help='logging level: %(choices)s')

    g.add_argument('--nprocs', metavar='N', type=int, default=0,
                   help='enable AthenaMP if %(metavar)s>=1 or %(metavar)s==-1')

    g.add_argument('--threads', metavar='N', type=int, default=0,
                   help='number of threads for AthenaMT, threads per worker for athenaMP')

    g.add_argument('--concurrent-events', metavar='N', type=int, default=0,
                   help='number of concurrent events for AthenaMT')

    g.add_argument('--mtes', action='store_true',
                   help='activate multi-threaded event service')

    g.add_argument('--mtes-channel', metavar='NAME', default='EventService_EventRanges',
                   help='yampl channel name between pilot and AthenaMT in event service mode')

    g.add_argument('-v', '--version', action='version', version=get_version(),
                   help='print version number')

    g.add_argument('-h', '--help', action='help',
                   help='show help message')

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('Run mode')

    g.add_argument('--CA', action='store_true',
                   help='ComponentAccumulator mode')

    g.add_argument('-i', '--interactive', action='store_true',
                   help='interactive mode')

    g.add_argument('--drop-and-reload', action='store_true', dest='drop_reload',
                   help='offload configuration and start new process')

    g.add_argument('--config-only', metavar='FILE', nargs='?', default=False, const=True,
                   help='run only configuration and optionally store in %(metavar)s')

    g.add_argument('--dump-configuration', metavar='FILE', dest='config_dump_file',
                   help='dump an ASCII version of the configuration to %(metavar)s')

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('Monitoring and debugging')

    g.add_argument('--perfmon', dest='pmon', action='store_const', const='perfmon',
                   help='enable performance monitoring toolkit (same as --pmon=perfmon)')

    g.add_argument('--pmon', metavar='MODE', action='store',
                   help='enable performance monitoring toolkit in MODE')

    g.add_argument('--profile-python', metavar='FILE',
                   help='profile python code, dump in %(metavar)s (.pkl or .txt)')

    g.add_argument('-d', '--debug', metavar='STAGE', nargs='?', const='init',
                   choices=['conf', 'init', 'exec', 'fini'],
                   help='attach debugger at stage: %(choices)s [%(const)s]')

    g.add_argument('--debugWorker', action='store_true', dest='debug_worker',
                   help='pause AthenaMP workers at bootstrap until SIGUSR1 signal received')

    g.add_argument('--leak-check', metavar='STAGE', dest='do_leak_chk', action=MemCheckAction,
                   choices=['initialize', 'start', 'beginrun', 'execute', 'finalize',
                            'endrun', 'stop', 'full', 'full-athena', 'all'],
                   help='perform basic memory leak checking, disables the use of tcmalloc.')

    g.add_argument('--delete-check', metavar='STAGE', dest='do_leak_chk', action=MemCheckAction,
                   choices=['initialize', 'start', 'beginrun', 'execute', 'finalize',
                            'endrun', 'stop', 'full', 'full-athena', 'all'],
                   help='perform double delete checking, disables the use of tcmalloc.')

    g.add_argument('-s', '--showincludes', action='store_true',
                   help='show printout of included files')

    g.add_argument('--trace', metavar='PATTERN', dest='trace_pattern',
                   help='also show files that match %(metavar)s')

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('System options')

    g.add_argument('--tcmalloc', action='store_true', dest='tcmalloc', default=True,
                   help='use tcmalloc.so for memory allocation [DEFAULT]')

    g.add_argument('--stdcmalloc', action='store_false', dest='tcmalloc',
                   help='use libc malloc for memory allocation')

    g.add_argument('--stdcmath', action='store_true', default=True,
                   help='use libc malloc for memory allocation [DEFAULT]')

    g.add_argument('--imf', action='store_true',
                   help='use Intel Math Function library')

    g.add_argument('--preloadlib', metavar='LIB',
                   help='localized preload of library %(metavar)s')

    g.add_argument('--enable-ers-hdlr', metavar='y/n', default='n', choices=['y','n'],
                   help='enable or not the ERS handler [%(default)s]')

    return parser


class AthOptionsError(SystemExit):
    def __init__(self, reason=None):
        import AthenaCommon.ExitCodes as ath_codes
        if reason is None:
            reason = ath_codes.OPTIONS_UNKNOWN
        try:
            message = ath_codes.codes[reason]
        except KeyError:
            message = ath_codes.codes[ath_codes.OPTIONS_UNKNOWN]

        SystemExit.__init__(self, reason, message)


def _help_and_exit(reason=None):
    raise AthOptionsError(reason)


def parse(chk_tcmalloc=True):
    """parses command line arguments and returns an ``Options`` instance"""

    # Everything after a single "-" is treated as "user options". This is for
    # backwards compatibility with AthArgumentParser used in analysis.
    # FIXME: we should revisit this and find an alternative
    try:
        dashpos = sys.argv.index("-")
    except ValueError:  # normal case, no isolated dash found
        args = sys.argv[1:]
        user_opts = []
    else:
        args = sys.argv[1:dashpos]
        user_opts = sys.argv[dashpos+1:]

    parser = getArgumentParser()
    opts = parser.parse_args(args)
    setattr(opts, 'user_opts', user_opts)

    set_environment(opts)
    check_tcmalloc(opts)
    fill_athenaCommonFlags(opts)

    return opts


if __name__ == '__main__':
    from pprint import pprint
    pprint(vars(parse()))
    print('TDAQ_ERS_NO_SIGNAL_HANDLERS', os.getenv('TDAQ_ERS_NO_SIGNAL_HANDLERS'))
