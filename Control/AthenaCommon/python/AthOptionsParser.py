# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# @file AthenaCommon.AthOptionsParser
# @purpose the central module to parse command line options of athena.py

import argparse
import os
import sys

# Are we running within athena.py ?
__athenaCLI = False

def enable_athenaCLI():
    """Enable athena-specific command linea arguments"""
    global __athenaCLI
    __athenaCLI = True


class JobOptAction(argparse.Action):
    """Check filename extension and fill relevant options"""
    def __call__(self, parser, args, values, option_string=None):
        scripts = [f  for f in values if f[-3:] == '.py']
        pkls    = [f  for f in values if f[-4:] == '.pkl']

        if (scripts and pkls) or len(pkls)>1:
            raise ValueError('Only job options or one pickle file is allowed')

        setattr(args, self.dest, scripts)
        args.fromdb = pkls[0] if pkls else None


class MemCheckAction(argparse.Action):
    """Enable Hepheastus"""
    def __call__(self, parser, args, values, option_string=None):

        setattr(args, self.dest,
                [] if values=='all' else [values])

        # early import is needed for proper offloading later
        import Hephaestus.MemoryTracker as memtrack  # noqa: F401

        if option_string=='--delete-check':
            args.memchk_mode = 'delete-check'
            import Hephaestus.DeleteChecker          # noqa: F401
        else:
            args.memchk_mode = 'leak-check'


class AthHelpFlags(argparse.Action):
    """Custom help action to support flags"""
    def __call__(self, parser, namespace, values, option_string=None):

        if not values:
            parser.print_help()
        else:
            import runpy
            sys.argv = ['athena.py', '--help']
            if values != 'flags':
                sys.argv.append(values)
            runpy.run_module('AthenaConfiguration.AthNoop', run_name='__main__')

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


def configureCAfromArgs(acc, opts):
    """Configure CA from relevant command line arguments if running from pkl"""

    if opts.interactive:
        acc.interactive = opts.interactive

    if opts.skipEvents:
        try:
            acc.getService('EventSelector').SkipEvents = opts.skipEvents
        except Exception:
            raise AthOptionsError("--skipEvents is not supported by this CA")

    if opts.debug:
        acc.setDebugStage(opts.debug)

    if opts.loglevel:
        from AthenaCommon import Constants
        acc.getService('MessageSvc').OutputLevel = getattr(Constants, opts.loglevel)


def getArgumentParser(legacy_args=False, **kwargs):
    """Create default argument parser"""

    parser = argparse.ArgumentParser(formatter_class=
                                     lambda prog : argparse.HelpFormatter(
                                         prog, max_help_position=40, width=100),
                                     add_help=False, **kwargs)

    parser.expert_groups = []   # List of expert option groups

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('Main options')

    if __athenaCLI and legacy_args:
        g.add_argument('scripts', nargs='*', action=JobOptAction,
                       help='scripts or pickle file to run')

    g.add_argument('-l', '--loglevel', metavar='LVL', type=str.upper, default='INFO',
                   choices=['ALL', 'VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR', 'FATAL'],
                   help='logging level: %(choices)s')

    g.add_argument('--filesInput', metavar='FILES',
                   help='set FilesInput property (comma-separated list with wildcards)')

    g.add_argument('--evtMax', metavar='N', type=int,
                   help='max number of events to process')

    g.add_argument('--skipEvents', metavar='N', type=int,
                   help='number of events to skip')

    g.add_argument('--nprocs', metavar='N', type=int,
                   help='enable AthenaMP if %(metavar)s>=1 or %(metavar)s==-1')

    g.add_argument('--threads', metavar='N', type=int,
                   help='number of threads for AthenaMT, threads per worker for athenaMP')

    g.add_argument('--concurrent-events', metavar='N', type=int,
                   help='number of concurrent events for AthenaMT')

    g.add_argument('--CA', action='store_true',
                   help='force ComponentAccumulator mode')

    g.add_argument('--config-only', metavar='FILE', nargs='?', default=False, const=True,
                   help='run only configuration and optionally store in %(metavar)s')

    g.add_argument('--mtes', action='store_true',
                   help='activate multi-threaded event service')

    g.add_argument('--mtes-channel', metavar='NAME', default='EventService_EventRanges',
                   help='yampl channel name between pilot and AthenaMT in event service mode')

    g.add_argument('--version', action='version', version=get_version(),
                   help='print version number')

    g.add_argument('-h', '--help', metavar='FLAGS', nargs='?', action=AthHelpFlags,
                   help='show help message (for FLAGS, "flags" for all categories)' if __athenaCLI
                   else 'show help message (for FLAGS category)')

    # --------------------------------------------------------------------------
    g = parser.add_argument_group('Monitoring and debugging')

    g.add_argument('--perfmon', metavar='MODE', nargs='?', const='fastmonmt',
                   help='enable performance monitoring toolkit in MODE')

    g.add_argument('-i', '--interactive', nargs='?', choices=['init', 'run'], const='init',
                   help='interactive mode with optional stage (default: init)')

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

    g.add_argument('--tracelevel', metavar='LEVEL', nargs='?', type=int, choices=range(1,4), const=3,
                   help='trace level for python configuration (%(choices)s)')

    # --------------------------------------------------------------------------
    if legacy_args:
        g = parser.add_argument_group('Legacy options')

        g.add_argument('-c', '--command', metavar='CMD',
                       help='one-liner, runs before any scripts')

        g.add_argument('--drop-and-reload', action='store_true', dest='drop_reload',
                       help='offload configuration and start new process')

        g.add_argument('--dump-configuration', metavar='FILE', dest='config_dump_file',
                       help='dump an ASCII version of the configuration to %(metavar)s')

        g.add_argument('-s', '--showincludes', action='store_true',
                       help='show printout of included files')

    # --------------------------------------------------------------------------
    if __athenaCLI:
        g = parser.add_argument_group('System options')

        g.add_argument('--tcmalloc', action='store_true', dest='tcmalloc', default=True,
                       help='use tcmalloc.so for memory allocation [DEFAULT]')

        g.add_argument('--stdcmalloc', action='store_false', dest='tcmalloc',
                       help='use libc malloc for memory allocation')

        g.add_argument('--stdcmath', action='store_true', default=True,
                       help='use libc malloc for memory allocation [DEFAULT]')

        g.add_argument('--imf', action='store_true',
                       help='use Intel Math Function library')

        g.add_argument('--exctrace', action='store_true',
                       help='preload exception trace collector')

        g.add_argument('--no-excabort', action='store_true',
                       help='disable converting some exceptions to abort')

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


def parse(legacy_args=False):
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

    parser = getArgumentParser(legacy_args)
    
    # perform a pre-parse without -h or --help
    # if don't have a script, then will just re-parse with help active
    # otherwise will effectively let script do the parsing
    # i.e. this allows the following to work:
    #   athena MyScript.py --help
    # to reveal the help messaging determined by MyScript.py
    doHelp = any(a in ('-h', '--help') or a.startswith('--help=') for a in args)
    if doHelp:
        # need to unrequire any required arguments in order to do a "pre-parse"
        unrequiredActions = []
        for a in parser._actions:
            if a.required:
                unrequiredActions.append(a)
                a.required = False

        # remove the help actions for the "pre-parse"
        helpActions = {'-h' : parser._option_string_actions.pop('-h'),
                       '--help' : parser._option_string_actions.pop('--help')}

        # parse with the modified parser
        opts, leftover = parser.parse_known_args(args)

        # restore original settings
        parser._option_string_actions.update(helpActions)
        for a in unrequiredActions:
            a.required = True

        # no script, just run argparsing as normal
        if not opts.scripts:
            opts, leftover = parser.parse_known_args(args)
    else:
        opts, leftover = parser.parse_known_args(args)

    opts.user_opts = user_opts

    # If the argument parser has been extended, the script name(s) may end up
    # in the leftovers. Try to find them there:
    if not (opts.scripts or opts.fromdb) and leftover:
        JobOptAction([], 'scripts')(parser, opts, leftover)

    if not (opts.scripts or opts.fromdb) and not opts.interactive:
        parser.error("the following arguments are required: scripts")

    set_environment(opts)
    check_tcmalloc(opts)

    return opts


if __name__ == '__main__':
    from pprint import pprint
    pprint(vars(parse()))
    print('TDAQ_ERS_NO_SIGNAL_HANDLERS', os.getenv('TDAQ_ERS_NO_SIGNAL_HANDLERS'))
