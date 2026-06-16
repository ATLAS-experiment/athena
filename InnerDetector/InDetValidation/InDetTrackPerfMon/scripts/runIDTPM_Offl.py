#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## Wrapper script for runIDTPM.py for offline analysis. It allows to run IDTPM
## in a simplified way, in the same fashion as IDPVM, i.e. without having to 
## prepare a json config file. Like in IDPVM, a baseline analysis is always scheduled
## with some default settings, and others may be enabled with specific flags, e.g. --doTightPrimary.
## All other flags from IDTPM are preserved.

import json
import subprocess
from argparse import ArgumentParser
from AthenaCommon.Logging import logging
log = logging.getLogger( "runIDTPM_Offl.py" )

class MyParser( ArgumentParser ):
    def exit( self, status=0, message=None ):
        if message: print(message, end='')  # print default help message
        # Add runIDTPM.py standard help message
        print( "\n\nStandard runIDTPM.py options:" )
        oldhelp = subprocess.run( ["runIDTPM.py", "-h"], capture_output=True, text=True ).stdout
        keep = False
        newhelp_list = []
        for line in oldhelp.splitlines():
            if keep :
                if "trkAnaCfgFile" in line or "track analysis setup" in line: continue
                newhelp_list.append(line)
            elif "options:" in line: keep = True
        newhelp = "\n".join( newhelp_list )
        print( f"{newhelp}" )
        super().exit( status )

def GetCustomAthArgs() :
    myparser = MyParser( description = 'runIDTPM_Offl.py options:',
                         usage = 'runIDTPM_Offl.py [runIDTPM_Offl.py options] [Standard runIDTPM.py options]' )
    myparser.add_argument( "-j", "--jsonName", help="json Config file name for IDTPM to write", default="IDTPMconfig.json" )
    myparser.add_argument( "-d", "--debug", help='set debug level printout', action='store_true', default=False )
    myparser.add_argument( "--doTightPrimary", help="Also schedule trackAnalysis with TightPrimary offline selection", action="store_true", default=False )
    # TODO - add here other flags for specific use cases
    return myparser.parse_known_args()

if __name__ == "__main__":
    ## Parsing arguments
    MyArgs, otherArgs = GetCustomAthArgs()
    if MyArgs.debug : log.setLevel( logging.DEBUG )

    ## default TrackAnalysis configuration
    IDPTM_json_config = {
        "TrkAnaOffl" : {
            "enabled" : True,
            "TestType"  : "Offline",
            "RefType"   : "Truth", 
            "MatchingType"  : "TruthMatch",
            "unlinkedAsFakes"  : False,
            "_comment"  : "unlinkedAsFakes=false is used for comparisons with IDPVM but the recommended default is true"
        }
    }

    ## if required add trackAnalyses to IDPTM_json_config
    if MyArgs.doTightPrimary:
        log.debug( "Adding TrkAnaOffl_TightPrimary" )
        trkAnaDict = IDPTM_json_config["TrkAnaOffl"].copy()
        trkAnaDict["OfflineQualityWP"] = "TightPrimary"
        trkAna_tightPrimary = { "TrkAnaOffl_TightPrimary" : trkAnaDict }
        IDPTM_json_config.update( trkAna_tightPrimary )
    ## TODO - add here other trackAnalyses configurations depending of the use-case

    with open( MyArgs.jsonName, "w", encoding="utf-8" ) as f:
        json.dump( IDPTM_json_config, f, indent=4, ensure_ascii=False )
        log.info( f"IDTPM config file {MyArgs.jsonName} with content" )
    subprocess.run( [ "jq", ".", MyArgs.jsonName ] )

    ## running IDTPM
    cmd = [ 'runIDTPM.py', 
            '--trkAnaCfgFile', MyArgs.jsonName ] + otherArgs
    log.debug( "Running: "+(' '.join( cmd )) )
    subprocess.run( cmd, check=True )
