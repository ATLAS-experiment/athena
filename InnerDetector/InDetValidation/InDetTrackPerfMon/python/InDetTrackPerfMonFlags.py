# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#from AthenaConfiguration.Enums import LHCPeriod

### IDTPM whole job properties
def __createIDTPMConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag( "DirName", "InDetTrackPerfMonPlots/" )
    icf.addFlag( "trkAnaNames", ["Default"] )
    icf.addFlag( "plotsDefFormat", "JSON" )
    icf.addFlag( "plotsDefFileList" , "InDetTrackPerfMon/HistoDefFileList_default.txt" )
    icf.addFlag( "plotsCommonValuesFile", "InDetTrackPerfMon/IDTPMPlotCommonValues.json" )
    icf.addFlag( "sortPlotsByChain", False )
    
    icf.addFlag( "trkAnaCfgFile", '' )
    icf.addFlag( 'outputFilePrefix','myIDTPM_out')
    icf.addFlag( 'unpackTrigChains', False )
    return icf


### IDTPM individual TrkAnalysis properties
### to be read from trkAnaCfgFile in JSON format
def __createIDTPMTrkAnaConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    # General properties
    icf.addFlag( "enabled", True )
    icf.addFlag( "anaTag", "" )
    icf.addFlag( "SubFolder", "" )
    # Test-Reference collections properties
    icf.addFlag( "TestType", "Offline" )
    icf.addFlag( "RefType", "Truth" )
    icf.addFlag( "TrigTrkKey"    , "HLT_IDTrack_Electron_IDTrig" )
    icf.addFlag( "OfflineTrkKey" , "InDetTrackParticles" )
    icf.addFlag( "TruthPartKey"  , "TruthParticles" )
    icf.addFlag( "pileupSwitch"  , "HardScatter" )
    # Matching properties
    icf.addFlag( "MatchingType"    , "DeltaRMatch" )
    icf.addFlag( "dRmax"           , 0.05 )
    icf.addFlag( "pTResMax"        , -9.9 )
    icf.addFlag( "truthProbCut"    , 0.5 )
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
    icf.addFlag( "OfflineQualityWP"   , "", help="Apply track quality selection cuts to the reconstructed tracks, if blank no selections is done" )
    icf.addFlag( "ObjectQuality"      , "Medium" )
    icf.addFlag( "TauType"            , "RNN" )
    icf.addFlag( "TauNprongs"         , 1 )
    icf.addFlag( "TruthProbMin"       , 0.5 )
    # ...
    # Truth particles selection properties
    # ...
    # Histogram properties
    icf.addFlag( "plotTrackParameters"      , True )
    icf.addFlag( "plotEfficiencies"         , True )
    icf.addFlag( "plotTechnicalEfficiencies", False )
    icf.addFlag( "plotResolutions"          , True )
    icf.addFlag( "plotFakeRates"            , True )
    icf.addFlag( "plotOfflineElectrons"     , False )
    icf.addFlag( "ResolutionMethod"         , "iterRMS" )
    
    return icf

### General config flag category for IDTPM tool job configuration
def initializeIDTPMConfigFlags(flags):
    flags.addFlagsCategory( "PhysVal.IDTPM",
                           __createIDTPMConfigFlags , prefix=True )
    
    flags.addFlag( 'Output.doWriteAOD_IDTPM', False )
    flags.addFlag( 'Output.AOD_IDTPMFileName', 'myIDTPM_out.AOD_IDTPM.pool.root' )
    return flags


### Create flags category and corresponding set of flags
def initializeIDTPMTrkAnaConfigFlags(flags):
    # Set output file names
    flags.PhysVal.OutputFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.HIST.root'
    flags.Output.AOD_IDTPMFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.AOD_IDTPM.pool.root'

    # Default TrackAnalysis configuration flags category
    flags.addFlagsCategory( "PhysVal.IDTPM.Default", 
                            __createIDTPMTrkAnaConfigFlags, 
                            prefix=True )
    
    from InDetTrackPerfMon.ConfigUtils import getTrkAnaDicts
    analysesDict = getTrkAnaDicts( flags )
    trkAnaNames = []
    print (str(analysesDict))

    if analysesDict:
        for trkAnaName, trkAnaDict in analysesDict.items():
            # Append TrkAnalysisName to list
            trkAnaNames.append( trkAnaName )

            # separate flag category for each TrkAnalysis
            flags.addFlagsCategory( "PhysVal.IDTPM."+trkAnaName, 
                                    __createIDTPMTrkAnaConfigFlags, 
                                    prefix=True )

            # set flags from values in trkAnaDict
            for fname, fvalue in trkAnaDict.items():
                setattr( flags.PhysVal.IDTPM, 
                        trkAnaName+"."+fname, fvalue )

    if trkAnaNames:
        flags.PhysVal.IDTPM.trkAnaNames = trkAnaNames
    return flags
