#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## quick script to get comma-separated list of files

import argparse

# Parsing arguments
parser = argparse.ArgumentParser( description = "MakePlots.py options:" )
parser.add_argument( "-s", "--sampleName", help="Short name of the sample to get" )

MyArgs = parser.parse_args()
sampleName = MyArgs.sampleName

## TODO - update this list with latest samples
## New samples added 20/5/25 JK
samplesDict = {
    "ttbar_pu200" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1",
    ],
    "ttbar_pu200_noFPT" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4422_r16130_tid41929901_00/RDO.41929901._000470.pool.root.1",
    ],
    "Zee_pu200" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.601189.PhPy8EG_AZNLO_Zee.recon.RDO.e8557_s4422_r16130_tid41978797_00/RDO.41978797._000284.pool.root.1",
    ],
    "Zmumu_pu200" : [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/input_RDOs/mc21_14TeV.601190.PhPy8EG_AZNLO_Zmumu.recon.RDO.e8557_s4422_r16130_tid41929981_00/RDO.41929981._000001.pool.root.1",
    ],
}

if sampleName in samplesDict:
    print( ','.join( samplesDict[ sampleName ] ) )
