#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## quick script to change the directory structure of a histogram file to that of another
## this is used to make an IDPVM output with the same format of IDTPM for comparison

import argparse, ROOT, os

# Parsing arguments
parser = argparse.ArgumentParser( description = "MakePlots.py options:" )
parser.add_argument( "-i", "--inputFile", help="IDPVM input file" )
parser.add_argument( "-c", "--config", help="config file" )
parser.add_argument( "-o", "--output", help="suffix for output file (IDTPMcnv for IDPVM vs IDTPM, ref for EF vs Offline)" )

MyArgs = parser.parse_args()

inFileName = MyArgs.inputFile
outputSuffix = MyArgs.output
inFile = ROOT.TFile.Open( inFileName , "READ" )

outFile = ROOT.TFile.Open( inFileName.replace( "root", outputSuffix+".root" ) , "RECREATE" )

configFileName = MyArgs.config
configFile = open( configFileName, 'r' )
lines = configFile.readlines()

for line in lines:
    parsed = line.strip().split()
    htype = parsed[0]
    href = parsed[1]
    htest = parsed[2]

    ## getting histo
    inFile.cd()
    obj = inFile.Get( href )
    obj.SetDirectory(0)

    ## writing histo
    outFile.cd()
    htest_dir = os.path.dirname( htest )
    htest_name = os.path.basename( htest )
    if( not outFile.GetDirectory( htest_dir ) ):
        outFile.mkdir( htest_dir, htest_dir )
    outFile.cd( htest_dir )
    obj.SetName( htest_name )
    obj.Write()

inFile.Close()
outFile.Close()
