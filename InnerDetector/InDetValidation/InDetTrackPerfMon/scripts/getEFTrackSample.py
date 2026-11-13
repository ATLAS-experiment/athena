#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## quick script to get comma-separated list of files
## This version now requires rucio to be defined to lookup the PFN

import sys, os.path, argparse, subprocess

# Parsing arguments
parser = argparse.ArgumentParser( description = "MakePlots.py options:" )
parser.add_argument( "-s", "--sampleName", help="Short name of the sample to get" )

MyArgs = parser.parse_args()
sampleName = MyArgs.sampleName

## TODO - update this list with ATLAS-P2-RUN4-05-00-00 samples
## New samples added 20/5/25 JK
## ATLAS-P2-RUN4-05-00-00 used for ttbar_pu200 12/11/26 TA
samplesDict = {
    "ttbar_pu200" : [
        "valid1:RDO.53187217._000001.pool.root.1", # from valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8586_s4680_r17882
    ],
    "ttbar_pu200_noFPT" : [
        "valid1:RDO.53187386._000001.pool.root.1", # from valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8586_s4680_r17880
    ],
    "Zee_pu200" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.601189.PhPy8EG_AZNLO_Zee.recon.RDO.e8557_s4422_r16130_tid41978797_00/RDO.41978797._000284.pool.root.1",
    ],
    "Zmumu_pu200" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.601190.PhPy8EG_AZNLO_Zmumu.recon.RDO.e8557_s4422_r16130_tid41929981_00/RDO.41929981._000001.pool.root.1",
    ],
    "singleMu_10GeV" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.900495.PG_single_muonpm_Pt10_etaFlatnp0_43.recon.RDO.e8557_s4422_r16128_tid41864947_00/RDO.41864947._000103.pool.root.1",
    ],
    "singleMu_100GeV" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8557_s4422_r16128_tid41864949_00/RDO.41864949._000001.pool.root.1",
    ]
}

def errorExit(rc, message):
    print(message, file=sys.stderr)
    sys.exit(rc)

prog = os.path.basename(sys.argv[0])
if sampleName not in samplesDict:
    errorExit( 1, f"{prog}: unrecognised sample name: {sampleName}" )
else:
    files=[]
    for fileName in samplesDict[ sampleName ]:
        if "/" in fileName:
            files.append( fileName )
        else:
            try:
                result = subprocess.run( ["rucio", "list-file-replicas", "--pfns", "--protocols", "root", fileName],
                                        stdout=subprocess.PIPE, text=True )
            except FileNotFoundError:
                errorExit( 2, f"{prog}: rucio not found. Use `lsetup rucio` before running {prog}" )
                raise  # keep linter happy, although we never get here
            if result.returncode != 0:
                errorExit( 3, f"{prog}: rucio list-file-replicas: failed with return code {result.returncode}" )
            pfns = result.stdout.splitlines()
            if not pfns:
                errorExit( 4, f"{prog}: rucio list-file-replicas: found no replicas for {fileName}" )
            files.append( pfns[0] )
        print( ','.join( files ) )
