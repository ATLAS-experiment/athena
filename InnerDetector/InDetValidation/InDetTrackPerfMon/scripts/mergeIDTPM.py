#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def GetCustomAthArgs() :
    from argparse import ArgumentParser
    myparser = ArgumentParser( description='Parser for IDTPM merger' )
    myparser.add_argument( "-i", "--inputFileNames", help="List of input files. Regex is allowed.", nargs='+', required=True )
    myparser.add_argument( "-o", "--outputFileName", help="Output file name", default='IDTPM.output.root' )
    myparser.add_argument( "-s", "--saveNonPostProcessed", help="Enable debugging messages", action="store_true", default=False )
    myparser.add_argument( "-m", "--method", help="Method for recomputing resolutions", choices=['iterRMS', 'gaussFit', 'iterRMSgaussFit'], default='iterRMS' )
    return myparser.parse_args()

## Parse the arguments
MyArgs = GetCustomAthArgs()

## Inputs
from glob import glob
InputFiles = []
for path in MyArgs.inputFileNames :
    InputFiles += glob( path )

import subprocess
## hadd inputs
cmd_1 = [ 'hadd', '-f', MyArgs.outputFileName ] + InputFiles
print( "Running: "+(' '.join( cmd_1 )) )
subprocess.run( cmd_1, check=True )

## Save hadd non-post-processed output
if MyArgs.saveNonPostProcessed :
    cmd_1a = [ 'cp', MyArgs.outputFileName,
               MyArgs.outputFileName.replace( ".root", "_nonPP.root" ) ]
    print( "Running: "+(' '.join( cmd_1a )) )
    subprocess.run( cmd_1a, check=True )

## Post-process output to recompute resolutions
cmd_2 = [ 'postProcessIDTPMHistos', MyArgs.outputFileName, MyArgs.method ]
print( "Running: "+(' '.join( cmd_2 )) )
subprocess.run( cmd_2, check=True )
