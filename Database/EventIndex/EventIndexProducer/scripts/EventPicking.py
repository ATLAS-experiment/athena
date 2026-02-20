#!/usr/bin/env python3

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

'''
@file EventPicking.py
@author ATLAS Computing Activity
@brief A thin wrapper around 'acmd filter-files' utility
'''

def main(args=None):
    import PyUtils.acmdlib as acmdlib
    import PyUtils.scripts   # noqa: F401  (register all plugins)

    cmd_name = 'filter-files'
    cmd = acmdlib.Plugins.load(cmd_name)
    if cmd is None:
        return 2

    # Parse user input
    import argparse
    parser = argparse.ArgumentParser(prog='EventPicking',
                                     description='Select events identified by run number (mc channel number'
                                     ' in case of Monte Carlo), event number from the input (POOL/BS) file(s).')
    parser.add_argument('--inputFiles', required=True,
                        help='input (POOL/BS) file(s) separated with commas')
    parser.add_argument('--outputFile', required=True,
                        help='output file')
    parser.add_argument('--eventList', required=True,
                        help='text file containing <run number> <event number> [<guid>] record(s) (one per line)')
    args, _ = parser.parse_known_args(args=args)

    # Convert our arguments to arguments expected by 'acmd filter-files' utility
    args.files = args.inputFiles.split(',')
    args.output = args.outputFile

    # Parse input file that contains run_number, event_number[, guid] combinations
    evtList = []
    with open(args.eventList) as f:
        for line in f:
            run, evt, *guid = line.rstrip().split()
            evtList.append((int(run), int(evt)))
    args.selection = repr(evtList)

    del args.inputFiles, args.outputFile, args.eventList

    return cmd.main(args)

# Main executable
if __name__ == "__main__":
    import sys
    sys.exit(main())
