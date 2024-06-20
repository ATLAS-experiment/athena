#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import argparse
import subprocess

def getArgs():
    args= argparse.ArgumentParser()
    args.add_argument('--inDS', required=True)
    args.add_argument('--outDS', required=True)
    args.add_argument('--nFiles', type=int)

    # We want to set these ones to %IN & %OUT...
    args.add_argument('--filesInput')
    args.add_argument('--outputFile')

    myArgs, otherArgs = args.parse_known_args()

    if myArgs.filesInput:
        print("ERROR: only specify --inDS, not --filesInput!")
        exit(1)

    if myArgs.outputFile:
        print("ERROR: only specify --outDS, not --outputFile!")
        exit(1)

    return myArgs, otherArgs


def main():

    args, otherArgs = getArgs()

    trf = [
        'runIDPVM.py',
        '--filesInput', '%IN',
        '--outputFile', '%OUT.IDPVM.root',
    ] + otherArgs
    trf = ' '.join(trf)

    merge = 'mergeIDPVM.py --filesInput %IN --outputFile %OUT'
    
    cmd = [
        'pathena',
        '--inDS', args.inDS,
        '--outDS', args.outDS,
        '--trf', trf,
        '--mergeOutput',
        '--mergeScript', merge
    ]

    if args.nFiles:
        cmd += ['--nFiles', str(args.nFiles)]

    subprocess.run(cmd, check=True)


if __name__ == '__main__':
    main()
