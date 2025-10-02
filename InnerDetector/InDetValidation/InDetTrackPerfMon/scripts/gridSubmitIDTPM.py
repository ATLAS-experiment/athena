#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def GetCustomAthArgs() :
    from argparse import ArgumentParser
    myparser = ArgumentParser( description='Parser for IDTPM merger' )
    myparser.add_argument( "-i", "--inDS", help="Input dataset", required=True )
    myparser.add_argument( "-o", "--outDS", help="Output dataset", required=True )
    myparser.add_argument( "-e", "--extraIDTPMOptions", help="other flags for runIDTPM.py, besides --inputFileNames and --outputFilePrefix", default='' )
    myparser.add_argument( "-m", "--merge", help="Merge output HIST files", action="store_true", default=False )
    myparser.add_argument( "-me", "--mergeExtraOptions", help="other flags for mergeIDTPM.py, besides -i and -o", default='' )
    return myparser.parse_known_args()

## Parse the arguments
MyArgs, otherPathenaArgs = GetCustomAthArgs()

import subprocess
## Main IDTPM transfrom
trf = [ 'runIDTPM.py',
        '--inputFileNames', '%IN',
        '--outputFilePrefix', '%OUT.IDTPM.HIST.root' ]
trf.append( MyArgs.extraIDTPMOptions )

## pathena command
cmd = [ 'pathena',
        '--inDS', MyArgs.inDS,
        '--outDS', MyArgs.outDS,
        '--trf', ' '.join(trf)
      ] + otherPathenaArgs

if MyArgs.merge :
    ## merge output script
    mergeCmd = [ 'mergeIDTPM.py', 
                 '-i', '%IN',
                 '-o', '%OUT'
               ]
    mergeCmd.append( MyArgs.mergeExtraOptions )

    cmd += [ '--mergeOutput',
             '--mergeScript', ' '.join( mergeCmd )
           ]

## submit
print( "Running: "+(' '.join( cmd )) )
subprocess.run( cmd, check=True )
