#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
"""Tool to check for error messages in a log file.

By default ERROR, FATAL and CRITICAL messages are considered.
The config file may be used to provide patterns of lines to exclude from this check
(known problems or false positives). If no config file is provided, all errors will be shown."""

import re
import argparse
import sys
import os

# Error keywords
regexMap = {}
regexMap['error/fatal'] = [
    r'^ERROR ', '^ERROR:', ' ERROR ', ' FATAL ', 'CRITICAL ', 'ABORT_CHAIN',
    r'^Exception\:',
    r'^Caught signal',
    r'^Core dump',
    r'tcmalloc\: allocation failed',
    r'athenaHLT.py\: error',
    r'HLTMPPU.*Child Issue',
    r'HLTMPPU.*Configuration Issue',
    r'There was a crash',
    r'illegal instruction',
    r'failure loading library',
    r'Cannot allocate memory',
    r'Attempt to free invalid pointer',
    r'CUDA error',
]

regexMap['prohibited'] = [
    r'inconsistent use of tabs and spaces in indentation',
    r'glibc detected',
    r'in state: CONTROLREADY$',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)missing data: ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)missing conditions data: ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)can be produced by alg\(s\): ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)required by tool: ',
    r'pure virtual method called',
    r'Selected dynamic Aux atribute.*not found in the registry',
]

regexMap['fpe'] = [
        r'FPEAuditor.*WARNING FPE',
]

# Add list of all builtin Python errors
import builtins
builtins = dir(builtins)
builtinErrors = [b for b in builtins if 'Error' in b]
regexMap['python error'] = builtinErrors

# Traceback keywords
backtrace = [
    r'Traceback',
    r'Shortened traceback',
    r'stack trace',
    r'^Algorithm stack',
    r'^#\d+\s*0x\w+ in ',
]
regexMap['backtrace'] = backtrace

# FPEAuditor traceback keywords
fpeTracebackStart = [r'FPEAuditor.*INFO FPE stacktrace']
fpeTracebackCont = [
    '  in function : ',
    '  included from : ',
    '  in library : ',
]
regexMap['fpe'].extend(fpeTracebackStart)

# Warning keywords
regexMap['warning'] = ['WARNING ']

for key,exprlist in regexMap.items():
    if not exprlist:
        raise RuntimeError(f'Empty regex list for category \'{key}\' -- will match everything!')
        sys.exit(1)

def get_parser():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=
                                     lambda prog : argparse.HelpFormatter(
                                         prog, max_help_position=40, width=100))

    parser.add_argument('logfile', metavar='<logfile>', nargs='+',
                        help='log file(s) to scan')
    parser.add_argument('--config', metavar='<file>',
                        help='specify config file')
    parser.add_argument('--showexcludestats', action='store_true',
                        help='print summary table with number of matches for each exclude pattern')
    parser.add_argument('--printpatterns', action='store_true',
                        help='print the list of warning/error patterns being searched for')
    parser.add_argument('--warnings', action = 'store_true',
                        help='check for WARNING messages')
    parser.add_argument('--errors', action = 'store_true',
                        help='check for ERROR messages')

    return parser


def main():
    parser = get_parser()

    args = parser.parse_args()
    if not (args.errors or args.warnings):
        parser.error('at least one of --errors or --warnings must be enabled')

    ignorePattern = parseConfig(args) if args.config else []
    rc = 0
    for i, lf in enumerate(args.logfile):
        if i>0:
            print()
        rc += scanLogfile(args, lf, ignorePattern)

    return rc


def parseConfig(args):
    """Parses the config file provided into a list (ignorePattern)"""
    ignorePattern = []

    os.system(f"get_files -data -symlink {args.config} > /dev/null")
    with open(args.config) as f:
        print('Ignoring warnings/error patterns defined in ' + args.config)
        for aline in f:
            if 'ignore' in aline:
              line = aline.strip('ignore').strip()
              if line.startswith('\'') and line.endswith('\''):
                  line = line[1:-1]
              ignorePattern.append(line)
    return ignorePattern


def scanLogfile(args, logfile, ignorePattern=[]):
    """Scan one log file and print report"""
    tPattern = re.compile('|'.join(backtrace))
    fpeStartPattern = re.compile('|'.join(fpeTracebackStart))
    fpeContPattern = re.compile('|'.join(fpeTracebackCont))
    ignoreDict = None

    categories = []
    if args.warnings is True:
        categories += ['warning']
    if args.errors is True:
        categories += ['error/fatal', 'prohibited', 'python error', 'fpe', 'backtrace']

    # if ignorePattern is empty, igLevels.search would match anything, protect by adding a pattern that matches nothing
    if not ignorePattern:
        ignorePattern.append('(?!)')

    igLevels = re.compile('|'.join(ignorePattern))

    patterns = {
        cat: re.compile('|'.join(regexMap[cat])) for cat in categories
    }
    resultsA = {cat:[] for cat in categories}
    with open(logfile, encoding='utf-8') as f:
        tracing = False
        fpeTracing = False

        for line in f:
            # First check if we need to start or continue following a trace
            # Tracing only makes sense for errors
            if args.errors:
                if tPattern.search(line) and not igLevels.search(line):
                    tracing = True
                elif fpeStartPattern.search(line) and not igLevels.search(line):
                    fpeTracing = True
                elif line =='\n':
                    tracing = False
                    fpeTracing = False

            if tracing:
                # Save all lines after a backtrace even if they don't belong to backtrace
                resultsA['backtrace'].append(line)
            elif fpeTracing:
                # Continue following FPE so long as recognised
                if fpeStartPattern.search(line) or fpeContPattern.search(line):
                    resultsA['fpe'].append(line)
                else:
                    fpeTracing = False
            else:
                for cat in categories:
                    if patterns[cat].search(line):
                        resultsA[cat].append(line)

    ignoreDict = {}
    results = {cat:[] for cat in categories}
    if args.config is None:
        results = resultsA
    else:
        if args.showexcludestats:
            separateIgnoreRegex = [re.compile(line) for line in ignorePattern]
            ignoreDict = {line:0 for line in ignorePattern} # stores counts of ignored errors/warnings

        # Filter messages
        for cat, messages in resultsA.items():
            for res in messages:
                if not igLevels.search(res):
                    results[cat].append(res)
                elif args.showexcludestats:
                    for i in range(len(separateIgnoreRegex)):
                        if separateIgnoreRegex[i].search(res):
                            ignoreDict[ignorePattern[i]] += 1


    # Report results
    found_bad_message = False
    for cat in categories:

        if args.printpatterns:
            print(f'check_log.py - Checking for {cat} messages with pattern: {str(patterns[cat])} in '+logfile+'\n')
        if len(results[cat]) > 0:
            print(f'Found {len(results[cat])} {cat} message(s) in {logfile}:')
            for msg in results[cat]: print(msg.strip('\n'))
            found_bad_message = True

    if ignoreDict:
        print('Ignored:')
        for s in ignoreDict:
            if ignoreDict[s] > 0:
                print(str(ignoreDict[s]) + "x " + s)
        print('\n')

    if found_bad_message:
        print(f'FAILURE : problematic message found in {logfile}')
        return 1

    print(f'No error/warning messages found in {logfile}')
    return 0


if __name__ == "__main__":
    sys.exit(main())
