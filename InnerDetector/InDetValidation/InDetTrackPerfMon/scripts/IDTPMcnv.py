#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## quick script to change the directory structure of a histogram file to that of another
## this is used to make an IDPVM output with the same format of IDTPM for comparison

import argparse, ROOT, os

# Parsing arguments
parser = argparse.ArgumentParser( description = "MakePlots.py options:" )
parser.add_argument( "-i", "--inputFile", help="IDPVM input file" )
parser.add_argument( "-c", "--config", help="config file" )

MyArgs = parser.parse_args()

inFileName = MyArgs.inputFile
inFile = ROOT.TFile.Open( inFileName , "READ" )

outFile = ROOT.TFile.Open( inFileName.replace( "root", "IDTPMcnv.root" ) , "RECREATE" )

configFileName = MyArgs.config
configFile = open( configFileName, 'r' )
lines = configFile.readlines()

for line in lines:
    parsed = line.strip().split()
    htype = parsed[0]
    hidpvm = parsed[1]
    hidtpm = parsed[2]

    ## getting histo
    inFile.cd()
    obj = inFile.Get( hidpvm )
    obj.SetDirectory(0)

    ## writing histo
    outFile.cd()
    hidtpm_dir = os.path.dirname( hidtpm )
    hidtpm_name = os.path.basename( hidtpm )
    if( not outFile.GetDirectory( hidtpm_dir ) ):
        outFile.mkdir( hidtpm_dir, hidtpm_dir )
    outFile.cd( hidtpm_dir )
    obj.SetName( hidtpm_name )
    obj.Write()

inFile.Close()
outFile.Close()
