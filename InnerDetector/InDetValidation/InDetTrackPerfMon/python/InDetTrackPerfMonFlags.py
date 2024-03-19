# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#from AthenaConfiguration.Enums import LHCPeriod

### IDTPM whole job properties
def createIDTPMConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag( "DirName", "InDetTrackPerfMonPlots/" )
    icf.addFlag( "trkAnaNames", ["Default"] )
    icf.addFlag( "histoDefFormat", "JSON" )
    icf.addFlag( "HistoDefFileList" , "InDetTrackPerfMon/HistoDefFileList_default.txt" )
    icf.addFlag( "plotsCommonValuesFile", "InDetTrackPerfMon/IDTPMPlotCommonValues.json" )
    
    return icf


### IDTPM individual TrkAnalysis properties
### to be read from trkAnaCfgFile in JSON format
def createIDTPMTrkAnaConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    # General properties
    icf.addFlag( "enabled", True )
    icf.addFlag( "anaTag", "" )
    icf.addFlag( "SubFolder", "IDTPM/" )
    # Test-Reference collections properties
    icf.addFlag( "TestType", "Offline" )
    icf.addFlag( "RefType", "Truth" )
    icf.addFlag( "TrigTrkKey"    , "HLT_IDTrack_Electron_IDTrig" )
    icf.addFlag( "OfflineTrkKey" , "InDetTrackParticles" )
    icf.addFlag( "TruthPartKey"  , "TruthParticles" )
    # Matching properties
    icf.addFlag( "MatchingType"    , "DeltaRMatch" )
    icf.addFlag( "dRmax"           , 0.05 )
    icf.addFlag( "pTResMax"        , -9.9 )
    # Trigger-specific properties
    icf.addFlag( "ChainNames"    , [] )
    icf.addFlag( "RoiKey"        , "" )
    icf.addFlag( "ChainLeg"      , -1 )
    icf.addFlag( "doTagNProbe"   , False )
    icf.addFlag( "RoiKeyTag"     , "" )
    icf.addFlag( "ChainLegTag"   , 0 )
    icf.addFlag( "RoiKeyProbe"   , "" )
    icf.addFlag( "ChainLegProbe" , 1 )
    # Offline tracks selection properties
    icf.addFlag( "SelectOfflineObject", "" )
    icf.addFlag( "ObjectQuality"      , "Medium" )
    icf.addFlag( "TauType"            , "RNN" )
    icf.addFlag( "TauNprongs"         , 1 )
    # ...
    # Truth particles selection properties
    # ...
    # Histogram properties
    icf.addFlag( "doTrackParameters"   , True )
    icf.addFlag( "doEfficiencies"      , True )
    icf.addFlag( "doOfflineElectrons"  , False )
    
    return icf
