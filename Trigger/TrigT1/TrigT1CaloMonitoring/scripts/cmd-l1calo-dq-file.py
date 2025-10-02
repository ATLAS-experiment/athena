#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import glob
import sys

if __name__=="__main__":

    import argparse
    parser = argparse.ArgumentParser(prog="l1calo-dq-file",description="Tries to obtain a path to a DQ HIST file",formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("run",help="run number")
    parser.add_argument("stream",nargs='?',default="express_express",help="stream name")
    parser.add_argument("--bulk",action='store_true',help="Look for bulk processing (f-tags, not x-tags)")
    args = parser.parse_args()
    files = glob.glob(f"/eos/atlas/atlastier0/rucio/*/{args.stream}/*{args.run}/*merge.HIST.{'f' if args.bulk else 'x'}*/*")
    if len(files)==1:
        print(files[0])
        exit(0)

    if len(files)==0:
        import subprocess
        # determine the project from folders
        folders = glob.glob(f"/eos/atlas/atlastier0/rucio/*/{args.stream}/*{args.run}")
        if len(folders)==0:
            print("Cannot determine project for run",args.run,file=sys.stderr)
            exit(1)
        project = folders[0].split("/")[5]
        pattern = f"{project}:{project}.{int(args.run):08}.{args.stream}.merge.HIST.{'f' if args.bulk else 'x'}*"
        try:
            datasets = [d for d in subprocess.run(["rucio","list-dids","--short", pattern],stdout=subprocess.PIPE).stdout.decode("utf-8").split('\n') if d !='']
        except FileNotFoundError:
            print("No file found. 'lsetup rucio' if you want to look further for the files",file=sys.stderr)
            exit(1)
        files = [f.split(",")[0] for f in subprocess.run(["rucio","list-files","--csv"]+datasets,stdout=subprocess.PIPE).stdout.decode("utf-8").split('\n') if f!='']
        if len(files)!=1:
            print("files:",*files,file=sys.stderr)
            exit(1)
        filePaths = [f[29:] for f in subprocess.run(["rucio", "list-file-replicas", "--pfns", "--rses", "CERN-PROD_DATADISK"]+files,stdout=subprocess.PIPE).stdout.decode("utf-8").split('\n')]
        filePaths = [f for f in filePaths if f != '']
        if len(filePaths)!=1:
            print("filePaths:",*filePaths,file=sys.stderr)
            exit(1)
        print(filePaths[0])
    else:
        print(*files)
