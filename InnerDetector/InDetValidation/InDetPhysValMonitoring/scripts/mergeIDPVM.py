#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import argparse
import glob
import subprocess

def getArgs():
    args= argparse.ArgumentParser()
    args.add_argument('--filesInput', required=True)
    args.add_argument('--outputFile', required=True)
    return args.parse_args()


def main():
    args = getArgs()

    inputs = []
    for path in args.filesInput.split(','):
        inputs += glob.glob(path)
        
        
    cmd_1 = ['hadd', args.outputFile] + inputs
    print(f"Running: {' '.join(cmd_1)}")
    subprocess.run(cmd_1, check=True)

    cmd_2 = ['postProcessIDPVMHistos', args.outputFile]
    print(f"Running: {' '.join(cmd_2)}")
    subprocess.run(cmd_2, check=True)


if __name__ == '__main__':
    main()
