#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# access the list of configured chains and expand the analysis chains 
# to be actual configued entries


# get the list of configured chains matching the regex 
# input parameter

import re


# get a chain configuration of the form "some_regex:key=Track:vtx=Vertex" etc
# parse to determine the trigger chain pattern, determine the chains
# matching the pattern, then recreate the full analysis string using all the
# matched chains - can take an individual analysis pattern, or a list of them

def getchains( flags, analyses, monlevel=None ):

    if isinstance( analyses, list ):
        a = analyses
    else:
        a = [ analyses ] 

    chains = []

    for analysis in a:
        if ":" in analysis:
        
            parts   = analysis.split( ":", 1 )    
            cchains = getconfiguredchains( flags, parts[0], monlevel )

            for c in cchains:
                if parts[1] is not None: 
                    chain = c + ":" + parts[1]
                else:
                    chain = c

                if chain not in chains:
                    chains.append( chain )

    return chains




def getconfiguredchains( flags, regex, monlevel=None ):
    if monlevel is None:
        return _getconfiguredchains( flags, regex )
    else:
        return _getmonchains( flags, regex, monlevel )



# cached full chain list

_chains = None

def _getconfiguredchains( flags, regex ):

    global _chains

    if _chains is None :
#       from datetime import datetime
#       print( datetime.now(), "getting trigger configuration" )

        from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess

        if flags is None:

            from AthenaConfiguration.AllConfigFlags import ConfigFlags
            flags = ConfigFlags

        _chains = getHLTMenuAccess( flags )

#       print( datetime.now(), "configured chains: ", len(_chains) )

    chains = []    
    for c in _chains:
        # print( c )
        chain = re.findall( regex, c )
        for a in chain: 
            if a is not None and c == a :
                chains.append( a )

    return chains




def _getmonchains( flags, regex, monlevel=None ):

    chains = []

    if monlevel is None: 
        return chains

    parts = monlevel.split( ":", 1 )    

    if parts[0] is None:
        return chains

    if parts[1] is None:
        return chains

    sig    = parts[0]
    levels = parts[1].split(":")

    _monchains = None

    if _monchains is None :

#       from datetime import datetime
#       print( datetime.now(), "getting trigger configuration" )

        from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess

        if flags is None:
            from AthenaConfiguration.AllConfigFlags import ConfigFlags
            flags = ConfigFlags

        moniAccess = getHLTMonitoringAccess(flags, filterOnActiveChains=True )

        _monchains = moniAccess.monitoredChains( signatures=sig, monLevels=levels )

#       print( datetime.now(), "configured chains: ", len(_monchains) )

    for c in _monchains:
        chain = re.findall( regex, c )
        for a in chain:
            if a is not None and c == a :
                chains.append( a )

    return chains


