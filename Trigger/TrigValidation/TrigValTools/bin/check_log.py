#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
errorRegex = [
    r'^ERROR ', ' ERROR ', ' FATAL ', 'CRITICAL ', 'ABORT_CHAIN',
    r'^Exception\:',
    r'^Caught signal',
    r'^Core dump',
    r'inconsistent use of tabs and spaces in indentation',
    r'glibc detected',
    r'tcmalloc\: allocation failed',
    r'athenaHLT.py\: error',
    r'HLTMPPU.*Child Issue',
    r'HLTMPPU.*Configuration Issue',
    r'There was a crash',
    r'illegal instruction',
    r'failure loading library',
    r'Cannot allocate memory',
    r'Attempt to free invalid pointer',
    r'in state: CONTROLREADY$',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)missing data: ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)missing conditions data: ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)can be produced by alg\(s\): ',
    r'(^\s*|^\d\d:\d\d:\d\d\s*)required by tool: ',
    r'pure virtual method called',
    r'Selected dynamic Aux atribute.*not found in the registry',
    r'FPEAuditor.*WARNING FPE',
    r'CUDA error',
]

# Add list of all builtin Python errors
builtins = dir(locals()['__builtins__'])
builtinErrors = [b for b in builtins if 'Error' in b]
errorRegex.extend(builtinErrors)

# Traceback keywords
traceback = [
    r'Traceback',
    r'Shortened traceback',
    r'stack trace',
    r'^Algorithm stack',
    r'^#\d+\s*0x\w+ in ',
]
errorRegex.extend(traceback)

# FPEAuditor traceback keywords
fpeTracebackStart = [r'FPEAuditor.*INFO FPE stacktrace']
fpeTracebackCont = [
    '  in function : ',
    '  included from : ',
    '  in library : ',
]
errorRegex.extend(fpeTracebackStart)

# Warning keywords
warningRegex = ['WARNING ']


def main():
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
    resultsA =[]
    pattern = []
    tPattern = re.compile('|'.join(traceback))
    fpeStartPattern = re.compile('|'.join(fpeTracebackStart))
    fpeContPattern = re.compile('|'.join(fpeTracebackCont))
    ignoreDict = {}

    if args.warnings is True:
        pattern = warningRegex
    if args.errors is True:
        pattern = errorRegex
    msgLevels = re.compile('|'.join(pattern))
    igLevels = re.compile('|'.join(ignorePattern))
    with open(logfile, encoding='utf-8') as f:
        tracing = False
        fpeTracing = False
        for line in f:
            #Tracing only makes sense for errors
            if args.errors is True and re.search(tPattern,line) and not re.search(igLevels,line):
                tracing = True
            elif args.errors is True and re.search(fpeStartPattern,line) and not re.search(igLevels,line):
                fpeTracing = True
            elif line =='\n':
                tracing = False
                fpeTracing = False
            if re.search(msgLevels,line):
                resultsA.append(line)
            elif tracing:
                # This currently prints all lines after a traceback even if they don't belong to traceback
                resultsA.append(line)
            elif fpeTracing:
                if re.search(fpeContPattern,line):
                    resultsA.append(line)
                else:
                    fpeTracing = False

    if args.showexcludestats and args.config:
        separateIgnoreRegex = [re.compile(line) for line in ignorePattern]
        ignoreDict = {line:0 for line in ignorePattern} # stores counts of ignored errors/warnings

    results = []
    if args.config is None:
        results = resultsA
    else:
        # Filter messages
        for res in resultsA:
            if not re.search(igLevels,res):
                results.append(res)
            elif args.showexcludestats:
                for i in range(len(separateIgnoreRegex)):
                    if re.search(separateIgnoreRegex[i],res):
                        ignoreDict[ignorePattern[i]] += 1

    # Report results
    if args.printpatterns:
        print('check_log.py - Checking for: '+ str(pattern) +' in '+logfile+'\n')
    if ignoreDict:
        print('Ignored:')
        for s in ignoreDict:
            if ignoreDict[s] > 0:
                print(str(ignoreDict[s]) + "x " + s)
        print('\n')
    if len(results) > 0:
        print(f'Found {len(results)} error/warning message(s) in {logfile}:')
        for msg in results: print(msg.strip('\n'))
        print(f'FAILURE : error/fatal found in {logfile}')
        return 1

    print(f'No error/warning messages found in {logfile}')
    return 0


if __name__ == "__main__":
    sys.exit(main())
