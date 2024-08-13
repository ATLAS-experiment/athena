# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import json
from AthenaCommon.Utils.unixtools import find_datafile
from AthenaCommon.Logging import logging

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
    if dataPath is None and input_file != "Default":
        raise Exception(f"Input file with analyses definition: {input_file} could not be found, for files given with absolute path use ./ prefix")
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
            ## Update SubFolder for this entry (= trkAnaName if not set or empty)
            if ( "SubFolder" not in analysesDict[trkAnaName].keys() or
                 analysesDict[trkAnaName]["SubFolder"] == "" ) :
                analysesDict[trkAnaName]["SubFolder"] = trkAnaName
            analysesDict[trkAnaName]["SubFolder"] += "/"
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


def getPlotsDefList( flags ):
    '''
    Open json files and load all merged contents 
    in a dictionary, which is later converted to a 
    list of strings, each to be parsed in a 
    (flattened) json format
    '''
    log = logging.getLogger( "getPlotsDefList" )

    # open the list of json files
    log.debug( "plotsDefFileList : %s", flags.PhysVal.IDTPM.plotsDefFileList ) 
    listPath = find_datafile( flags.PhysVal.IDTPM.plotsDefFileList )
    if listPath is None:
        log.error( "plotsDefFileList not found" )
        return None

    plotsDefFileNames = []
    with open( listPath, "r" ) as input_flist :
        plotsDefFileNames = input_flist.read().splitlines()

    # creating the basic histogrm definition dictionary 
    plotsDefDict = {}

    for plotsDefFileName in plotsDefFileNames :
        dataPath = find_datafile( plotsDefFileName )
        log.debug( "Reading input plots definitions : %s", dataPath )
        if dataPath is None:
            log.error( "plotsDefFile %s not found", plotsDefFileName )
            return None

        with open( dataPath, "r" ) as input_json_file :
            plotsDefDict.update( json.load( input_json_file ) )

    # Turn all histo definitions into a list of strings
    # each string has a flattened json format
    plotsDefStrList_v1 = []
    for plotName, plotDict in plotsDefDict.items():
        newPlotDict = plotDict.copy()
        newPlotDict[ "name" ] = plotName

        # flatten json histo dict
        plotDictFlat = flatten_json( newPlotDict )

        # Turn json into string
        plotDefStr = str( json.dumps( plotDictFlat ) )

        # append to list
        plotsDefStrList_v1.append( plotDefStr )

    # Replace standard common fields (e.g. &ETAMAX)
    # with corresponding values (read from default json)
    commonValuesDict = {}
    commonValuesPath = find_datafile( flags.PhysVal.IDTPM.plotsCommonValuesFile )
    if commonValuesPath is None :
        log.error( "plotsCommonValuesFile not found" )
        return None

    with open( commonValuesPath, "r" ) as input_commonValues : 
        commonValuesDict.update( json.load( input_commonValues ) )

    plotsDefStrList_v2 = []
    for plotDefStr in plotsDefStrList_v1 :
        newPlotDefStr = plotDefStr
        if commonValuesDict :
            for key, value in commonValuesDict.items() :
                plotDefStr_tmp = newPlotDefStr.replace( "$"+key, value[0] )
                newPlotDefStr = plotDefStr_tmp
        plotsDefStrList_v2.append( newPlotDefStr )

    # Now expand the list to account for all required track types
    testLabel = getLabel( flags, flags.PhysVal.IDTPM.currentTrkAna.TestType )
    refLabel  = getLabel( flags, flags.PhysVal.IDTPM.currentTrkAna.RefType )
    trkLabels = [ testLabel, refLabel ]

    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "EFTruthMatch":
        trkLabels.append( getLabel( flags, "Truth" ) )

    plotsDefStrList = []
    for plotsDefStr in plotsDefStrList_v2 :
        plotsDefStr = plotsDefStr.replace( "$TESTTYPE", testLabel[0] ).replace( "$TESTTAG", testLabel[1] )
        plotsDefStr = plotsDefStr.replace( "$REFTYPE", refLabel[0] ).replace( "$REFTAG", refLabel[1] )
        if ( "$TRKTAG" not in plotsDefStr ) and ( "$TRKTYPE" not in plotsDefStr ) :
            plotsDefStrList.append( plotsDefStr )
            continue
        for trkLabel in trkLabels :
            newPlotsDefStr = plotsDefStr.replace( "$TRKTYPE", trkLabel[0] ).replace( "$TRKTAG", trkLabel[1] )
            plotsDefStrList.append( newPlotsDefStr )

    return plotsDefStrList


def getLabel( flags, key ) :
    if key == "Offline" and flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject:
        if "Truth" not in flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject:
            key += flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject
    trkLabelsDict = {
        "EFTrigger"           : [ "EF Trigger track",  "eftrig" ],
        "Trigger"             : [ "Trigger track",  "trig" ],
        "Offline"             : [ "Offline track",              "offl" ],
        "OfflineElectron"     : [ "Offline e^{#pm} track",      "offEle" ],
        "OfflineMuon"         : [ "Offline #mu^{#pm} track",    "offMu" ],
        "OfflineTau"          : [ "Offline #tau^{#pm} track",   "offTau" ],
        "Truth"               : [ "Truth particle",     "truth" ],
        "TruthElectron"       : [ "Truth e^{#pm}",      "truthEle" ],
        "TruthMuon"           : [ "Truth #mu^{#pm}",    "truthMu" ],
        "TruthTau"            : [ "Truth #tau^{#pm}",   "truthEle" ],
    }
    return trkLabelsDict[ key ]


def getTag( flags, key ) :
    labels = getLabel( flags, key )
    return labels[1]


def flatten_json( y ) :
    out = {}
    def flatten(x, name=''):
        if type(x) is dict:
            for a in x:
                flatten(x[a], name + a + '_')
        else:
            out[name[:-1]] = x
    flatten(y)
    return out
