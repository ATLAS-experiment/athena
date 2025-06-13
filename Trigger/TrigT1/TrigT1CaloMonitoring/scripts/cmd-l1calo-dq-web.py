#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    import argparse
    parser = argparse.ArgumentParser(prog="l1calo-dq-web",description="Generate a WebDisplay website for a run, using the local DataQuality configuration",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("runNumber",help="run number to create website for")
    parser.add_argument("stream",default="express_express",nargs="?",help="stream to create for")
    parser.add_argument("--bulk",action='store_true',help="Use bulk processing")

    args = parser.parse_args()

    import glob
    files = glob.glob(f"/eos/atlas/atlastier0/rucio/*/{args.stream}/*{args.runNumber}/*merge.HIST.{'f' if args.bulk else 'x'}*/*")
    if len(files)==0:
        print("ERROR: No HIST file found for run")
        exit(1)
    elif len(files)!=1:
        print("ERROR: Multiple HIST files found:",*files)
        exit(1)

    import random
    r = random.randrange(100000,999999)
    cmdStr = f"DQWebDisplay.py {files[0]} TestDisplay \"{r}\""

    import subprocess
    print("Executing:",cmdStr, "... Please be patient ...")
    process = subprocess.Popen(cmdStr, shell=True)
    process.wait()
    if process.returncode==0:
        urlStr = f"https://atlasdqm.cern.ch/webdisplay/test/{r}/{args.stream}/run_{args.runNumber}/run/"
        print("WebDisplay successfully generated:",urlStr)
    else:
        print("ERROR: WebDisplay generation failed")
        exit(1)
