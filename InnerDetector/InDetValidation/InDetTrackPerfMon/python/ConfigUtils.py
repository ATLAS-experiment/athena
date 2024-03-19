# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import json
from AthenaCommon.Utils.unixtools import find_datafile

def getTrkAnaDicts( flags, input_file, unpackChains=False ):
    '''
    utility function to retrieve the flag dictionary
    for every TrackAnalysis from an input JSON file
    '''
    analysesDict = {}

    ## Default: use trkAnalysis config flags
    if input_file == "Default":
        return analysesDict

    ## Getting full input json file path
    dataPath = find_datafile( input_file )
    if dataPath is None:
        return analysesDict

    ## Fill temporary analyses config dictionary from input json
    analysesDictTmp = {}
    with open( dataPath, "r" ) as input_json_file:
        analysesDictTmp = json.load( input_json_file )

    ## Update each analysis dictionary with their anaTag and ChainNames list
    if analysesDictTmp:
        for trkAnaName, trkAnaDict in analysesDictTmp.items():
            ## Update entry with flags read from json
            analysesDict.update( { trkAnaName : trkAnaDict } )
            ## Update TrkAnalysis tag for this entry
            analysesDict[trkAnaName]["anaTag"] = "_" + trkAnaName
            ## Update TrkAnalysis ChainNames to full chain list (not regex)
            if "ChainNames" in analysesDict[trkAnaName]:
                fullChainList = getChainList( flags, analysesDict[trkAnaName]["ChainNames"] )
                analysesDict[trkAnaName]["ChainNames"] = fullChainList

    return unpackTrkAnaDicts( analysesDict ) if unpackChains else analysesDict


def unpackTrkAnaDicts( analysesDictIn ):
    '''
    utility function to define a separate TrackAnalysis
    for each configured trigger chain
    '''
    if not analysesDictIn : return analysesDictIn

    analysesDictOut = {}
    for trkAnaName, trkAnaDict in analysesDictIn.items():
        if "ChainNames" in trkAnaDict:
            chainList = trkAnaDict["ChainNames"]
            for chain in chainList:
                trkAnaName_new = trkAnaName + "_" + chain
                trkAnaDict_new = dict( trkAnaDict )
                trkAnaDict_new["anaTag"] = trkAnaDict["anaTag"] + "_" + chain
                trkAnaDict_new["ChainNames"] = [ chain ]
                analysesDictOut.update( { trkAnaName_new : trkAnaDict_new } )
        else:
            analysesDictOut.update( { trkAnaName : trkAnaDict } )

    return analysesDictOut



def getChainList( flags, regexChainList=[] ):
    '''
    utility function to retrieve full list of
    configured trigger chains matching
    the passed regex list of chains
    '''
    if not regexChainList: return regexChainList

    if not flags.locked():
        flags_tmp = flags.clone()
        flags_tmp.lock()
        flags = flags_tmp

    from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
    chainsMenu = getHLTMenuAccess( flags )

    import re
    configChains = []
    for regexChain in regexChainList:
        for item in chainsMenu:
            chains = re.findall( regexChain, item )
            for chain in chains:
                if chain is not None and chain == item:
                    configChains.append( chain )

    return configChains
