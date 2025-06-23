# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import json, os
from AthenaCommon.Utils.unixtools import find_datafile
from AthenaCommon.Logging import logging

def custom_find_datafile( input_file_name ):
    '''
    wrapper around AthenaCommon.Utils.unixtools.find_datafile
    to handle local paths in the same running directory, by
    returning the absolute path ( thisFile.json -> /path_to/thisFile.json )
    '''
    new_file_name = input_file_name
    if not os.path.dirname( new_file_name ) :
        new_file_name = os.path.abspath( new_file_name ) # attach absolute path
    return find_datafile( new_file_name )


def getTrkAnaDicts( flags ):
    '''
    utility function to retrieve the flag dictionary
    for every TrackAnalysis from an input JSON file
    '''
    analysesDict = {}

    ## Default: use trkAnalysis default config flags
    if flags.PhysVal.IDTPM.trkAnaCfgFile == "Default":
        return analysesDict

    ## Getting full input json file path
    dataPath = custom_find_datafile( flags.PhysVal.IDTPM.trkAnaCfgFile )
    if dataPath is None:
        raise Exception(f"Input file with analyses definition: {dataPath} could not be found")
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

    return unpackTrkAnaDicts( analysesDict ) if flags.PhysVal.IDTPM.unpackTrigChains else analysesDict


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
    listPath = custom_find_datafile( flags.PhysVal.IDTPM.plotsDefFileList )
    if listPath is None:
        log.error( "plotsDefFileList not found" )
        return None

    plotsDefFileNames = []
    with open( listPath, "r" ) as input_flist :
        plotsDefFileNames = input_flist.read().splitlines()

    # creating the basic histogrm definition dictionary 
    plotsDefDict = {}

    for plotsDefFileName in plotsDefFileNames :
        dataPath = custom_find_datafile( plotsDefFileName )
        log.debug( "Reading input plots definitions : %s", dataPath )
        if dataPath is None:
            log.error( "plotsDefFile %s not found", plotsDefFileName )
            return None

        with open( dataPath, "r" ) as input_json_file :
            plotsDefDict.update( json.load( input_json_file ) )

    # Expand plots definitions for resolutions and pulls
    # from corresponding TH2F Helpers
    plotsDefDict = updateResolutionPlots( plotsDefDict )

    # Turn all histo definitions into a list of strings
    # each string has a flattened json format
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
    plotsCommonValuesFileName = flags.PhysVal.IDTPM.plotsCommonValuesFile
    if not plotsCommonValuesFileName :
        if flags.Detector.GeometryID :
            plotsCommonValuesFileName = "InDetTrackPerfMon/PlotsDefCommonValues.json"
        elif flags.Detector.GeometryITk :
            plotsCommonValuesFileName = "InDetTrackPerfMon/PlotsDefCommonValues_ITk.json"
        else :
            log.error( "Could not get detector geometry for plotsCommonValuesFile" )
            return None

    commonValuesPath = custom_find_datafile( plotsCommonValuesFileName )
    if commonValuesPath is None :
        log.error( "plotsCommonValuesFile not found: %s", plotsCommonValuesFileName )
        return None

    commonValuesDict = {}
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

    testAllLabel = getAllTruthLabel( flags, flags.PhysVal.IDTPM.currentTrkAna.TestType )
    refAllLabel  = getAllTruthLabel( flags, flags.PhysVal.IDTPM.currentTrkAna.RefType )


    if flags.PhysVal.IDTPM.currentTrkAna.MatchingType == "EFTruthMatch":
        trkLabels.append( getLabel( flags, "Truth" ) )

    ## First loop to replace track tags and labels
    plotsDefStrList = []
    for plotsDefStr in plotsDefStrList_v2 :
        plotsDefStr = plotsDefStr.replace( "$TESTALL", testAllLabel ).replace( "$REFALL", refAllLabel )
        plotsDefStr = plotsDefStr.replace( "$TESTTYPE", testLabel[1] ).replace( "$TESTTAG", testLabel[0] )
        plotsDefStr = plotsDefStr.replace( "$REFTYPE", refLabel[1] ).replace( "$REFTAG", refLabel[0] )
        if ( "$TRKTAG" not in plotsDefStr ) and ( "$TRKTYPE" not in plotsDefStr ) :
            plotsDefStrList.append( plotsDefStr )
            continue
        for trkLabel in trkLabels :
            newPlotsDefStr = plotsDefStr.replace( "$TRKTYPE", trkLabel[1] ).replace( "$TRKTAG", trkLabel[0] )
            if ( "$TRK2TAG" not in newPlotsDefStr ) and ( "$TRK2TYPE" not in newPlotsDefStr ) :
                plotsDefStrList.append( newPlotsDefStr )
                continue
            for trk2Label in trkLabels :
                newPlotsDefStr2 = newPlotsDefStr.replace( "$TRK2TYPE", trk2Label[1] ).replace( "$TRK2TAG", trk2Label[0] )
                plotsDefStrList.append( newPlotsDefStr2 )

    ## Second loop to replace vertex tags and labels
    plotsDefStrList_v2 = []
    for plotsDefStr in plotsDefStrList :
        plotsDefStr = plotsDefStr.replace( "$TESTVTXTYPE", testLabel[2] ).replace( "$TESTVTXTAG", testLabel[0] )
        plotsDefStr = plotsDefStr.replace( "$REFVTXTYPE", refLabel[2] ).replace( "$REFVTXTAG", refLabel[0] )
        if ( "$VTXTAG" not in plotsDefStr ) and ( "$VTXTYPE" not in plotsDefStr ) :
            plotsDefStrList_v2.append( plotsDefStr )
            continue
        for trkLabel in trkLabels :
            newPlotsDefStr = plotsDefStr.replace( "$VTXTYPE", trkLabel[2] ).replace( "$VTXTAG", trkLabel[0] )
            if ( "$VTX2TAG" not in newPlotsDefStr ) and ( "$VTX2TYPE" not in newPlotsDefStr ) :
                plotsDefStrList_v2.append( newPlotsDefStr )
                continue
            for trk2Label in trkLabels :
                newPlotsDefStr2 = newPlotsDefStr.replace( "$VTX2TYPE", trk2Label[2] ).replace( "$VTX2TAG", trk2Label[0] )
                plotsDefStrList_v2.append( newPlotsDefStr2 )

    return plotsDefStrList_v2


def getLabel( flags, key ) :
    if key == "Offline" and flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject:
        if "Truth" not in flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject:
            key += flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject
    trkLabelsDict = {
        # key                 : [ trk/vtx_tag,  track_type,                 vertex_type                 ]
        "EFTrigger"           : [ "eftrig",     "EF Trigger track",         "EF Trigger vertex"         ],
        "Trigger"             : [ "trig",       "Trigger track",            "Trigger vertex"            ],
        "Offline"             : [ "offl",       "Offline track",            "Offline vertex"            ],
        "OfflineElectron"     : [ "offEle",     "Offline e^{#pm} track",    "Offline e^{#pm} vertex"    ],
        "OfflineMuon"         : [ "offMu",      "Offline #mu^{#pm} track",  "Offline #mu^{#pm} vertex"  ],
        "OfflineTau"          : [ "offTau",     "Offline #tau^{#pm} track", "Offline #tau^{#pm} vertex" ],
        "Truth"               : [ "truth",      "Truth particle",           "Truth vertex"              ],
        "TruthElectron"       : [ "truthEle",   "Truth e^{#pm}",            "Truth e^{#pm} vertex"      ],
        "TruthMuon"           : [ "truthMu",    "Truth #mu^{#pm}",          "Truth #mu^{#pm} vertex"    ],
        "TruthTau"            : [ "truthTau",   "Truth #tau^{#pm}",         "Truth #tau^{#pm} vertex"   ]
    }
    return trkLabelsDict[ key ]


def getTag( flags, key ) :
    labels = getLabel( flags, key )
    return labels[0]


def updateResolutionPlots( myPlotsDefDict ) :
    # initialize output dict to input
    outDict = myPlotsDefDict.copy()

    items = [ "res", "pull" ]
    plist = [ "mean", "width" ]
    iDict = {
        "res" : {
            "helper" : { "name" : "resHelper",  "yTitle" : "residual" },
            "mean"   : { "name" : "resmean",    "yTitle" : "bias" },
            "width"  : { "name" : "resolution", "yTitle" : "resolution" }
        },
        "pull" : {
            "helper" : { "name" : "pullHelper", "yTitle" : "pull" },
            "mean"   : { "name" : "pullmean",   "yTitle" : "pull mean" },
            "width"  : { "name" : "pullwidth",  "yTitle" : "pull width" }
        }
    }

    for plotName, plotDict in myPlotsDefDict.items() :
        # processing resHelpers and pullHelpers
        for i in items:
            if iDict[i]["helper"]["name"] in plotName :
                # processing mean and width plots
                for p in plist :
                    # Computing the derived plot (mean or width) name from helper
                    pName = plotName.replace( iDict[i]["helper"]["name"], iDict[i][p]["name"] )

                    pDict = {}
                    if pName in outDict :
                        # plot definition already exists. Grabbing the original
                        pDict = outDict[ pName ]
                    else :
                        # plot definition doesn't exist.
                        # Computing and setting yAxis title from helper's yAxis'
                        # (e.g. "VAR residual [unit]" -> "VAR resolution [unit]")
                        yTitle = plotDict["yAxis"]["title"]
                        yTitle = yTitle.replace( iDict[i]["helper"]["yTitle"], iDict[i][p]["yTitle"] )
                        pDict.update( { "yAxis" : { "title" : yTitle } } )

                    # forcing x-axis to be the same as helper's (in all cases) and type = TH1F
                    pDict.update( {
                        "type" : "TH1F",
                        "xAxis" : plotDict["xAxis"]
                    } )

                    # update outdict
                    outDict.update( { pName : pDict } )

    return outDict


def getAllTruthLabel( flags, key ) :
    ## replace All label to All HS/PU depending
    ## pileupSwitch, for truth particles
    if key != "Truth":
        return "All"
    if flags.PhysVal.IDTPM.currentTrkAna.pileupSwitch == "HardScatter" :
        return "All HS"
    if flags.PhysVal.IDTPM.currentTrkAna.pileupSwitch == "PileUp" :
        return "All PU"
    return "All"
