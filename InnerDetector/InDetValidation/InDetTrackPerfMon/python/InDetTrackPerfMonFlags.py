# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#from AthenaConfiguration.Enums import LHCPeriod

### IDTPM whole job properties
def __createIDTPMConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag( "DirName", "InDetTrackPerfMonPlots/" )
    icf.addFlag( "trkAnaNames", ["Default"] )
    icf.addFlag( "plotsDefFormat", "JSON" )
    icf.addFlag( "plotsDefFileList" , "InDetTrackPerfMon/PlotsDefFileList_default.txt" )
    icf.addFlag( "plotsCommonValuesFile", "" )
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
    icf.addFlag( "DoOfflineSelection", False )
    icf.addFlag( "offlMaxZ0SinTheta",               -9999. )
    icf.addFlag( "offlMinNInnermostLayerHits",      -9999. )
    icf.addFlag( "offlMinNBothInnermostLayersHits", -9999. )
    icf.addFlag( "offlMaxNInnermostLayerSharedHits",-9999. )
    icf.addFlag( "offlMinNSiHits",                  -9999. )
    icf.addFlag( "offlMaxNSiSharedHits",            -9999. )
    icf.addFlag( "offlMaxNSiHoles",                 -9999. )
    icf.addFlag( "offlMinNPixelHits",               -9999. )
    icf.addFlag( "offlMaxNPixelSharedHits",         -9999. )
    icf.addFlag( "offlMaxNPixelHoles",              -9999. )
    icf.addFlag( "offlMinNSctHits",                 -9999. )
    icf.addFlag( "offlMaxNSctSharedHits",           -9999.)
    icf.addFlag( "offlMaxNSctHoles",                -9999. )
    icf.addFlag( "offlMaxChiSq",                    -9999. )
    icf.addFlag( "offlMaxChiSqperNdf",              -9999. )
    icf.addFlag( "offlMinPt"   , -9999.)
    icf.addFlag( "offlMaxPt"   , -9999. )
    icf.addFlag( "offlMinEta"  , -9999. )
    icf.addFlag( "offlMaxEta"  , -9999. )
    icf.addFlag( "offlMinPhi"  , -9999. )
    icf.addFlag( "offlMaxPhi"  , -9999. )
    icf.addFlag( "offlMinD0"   , -9999. )
    icf.addFlag( "offlMaxD0"   , -9999. )
    icf.addFlag( "offlMinZ0"   , -9999. )
    icf.addFlag( "offlMaxZ0"   , -9999. )
    icf.addFlag( "offlMinQoPT" , -9999. )
    icf.addFlag( "offlMaxQoPT" , -9999. )
    icf.addFlag( "offlMaxAbsEta"  , -9999. )
    icf.addFlag( "offlMinAbsEta"  , -9999. )
    icf.addFlag( "offlMinAbsPhi"  , -9999. )
    icf.addFlag( "offlMaxAbsPhi"  , -9999. )
    icf.addFlag( "offlMinAbsD0"   , -9999. )
    icf.addFlag( "offlMaxAbsD0"   , -9999. )
    icf.addFlag( "offlMinAbsZ0"   , -9999. )
    icf.addFlag( "offlMaxAbsZ0"   , -9999. )
    icf.addFlag( "offlMinAbsQoPT" , -9999. )
    icf.addFlag( "offlMaxAbsQoPT" , -9999. )
    icf.addFlag( "offlMinProb",                     -9999. )
    icf.addFlag( "ObjectQuality"      , "Medium" )
    icf.addFlag( "TauType"            , "RNN" )
    icf.addFlag( "TauNprongs"         , 1 )
    icf.addFlag( "TruthProbMin"       , 0.5 )
    # Truth particles selection properties
    icf.addFlag( "SelectTruthObject"   , "" )
    icf.addFlag( "truthMinPt"   , -9999., help="Apply minimum pt cut to truth particle" )
    icf.addFlag( "truthMaxPt"   , -9999., help="Apply maximum pt cut to truth particle" )
    icf.addFlag( "truthMinEta"  , -9999., help="Apply minimum eta cut to truth particle" )
    icf.addFlag( "truthMaxEta"  , -9999., help="Apply maximum eta cut to truth particle" )
    icf.addFlag( "truthMinPhi"  , -9999., help="Apply minimum phi cut to truth particle" )
    icf.addFlag( "truthMaxPhi"  , -9999., help="Apply maximum phi cut to truth particle" )
    icf.addFlag( "truthMinD0"   , -9999., help="Apply minimum d0 cut to truth particle" )
    icf.addFlag( "truthMaxD0"   , -9999., help="Apply maximum d0 cut to truth particle" )
    icf.addFlag( "truthMinZ0"   , -9999., help="Apply minimum z0 cut to truth particle" )
    icf.addFlag( "truthMaxZ0"   , -9999., help="Apply maximum z0 cut to truth particle" )
    icf.addFlag( "truthMinQoPT" , -9999., help="Apply minimum q/pt cut to truth particle" )
    icf.addFlag( "truthMaxQoPT" , -9999., help="Apply maximum q/pt cut to truth particle" )
    icf.addFlag( "truthMinAbsEta"  , -9999., help="Apply minimum |eta| cut to truth particle" )
    icf.addFlag( "truthMaxAbsEta"  , -9999., help="Apply maximum |eta| cut to truth particle" )
    icf.addFlag( "truthMinAbsPhi"  , -9999., help="Apply minimum |phi| cut to truth particle" )
    icf.addFlag( "truthMaxAbsPhi"  , -9999., help="Apply maximum |phi| cut to truth particle" )
    icf.addFlag( "truthMinAbsD0"   , -9999., help="Apply minimum |d0| cut to truth particle" )
    icf.addFlag( "truthMaxAbsD0"   , -9999., help="Apply maximum |d0| cut to truth particle" )
    icf.addFlag( "truthMinAbsZ0"   , -9999., help="Apply minimum |z0| cut to truth particle" )
    icf.addFlag( "truthMaxAbsZ0"   , -9999., help="Apply maximum |z0| cut to truth particle" )
    icf.addFlag( "truthMinAbsQoPT" , -9999., help="Apply minimum |q/pt| cut to truth particle" )
    icf.addFlag( "truthMaxAbsQoPT" , -9999., help="Apply maximum |q/pt| cut to truth particle" )
    icf.addFlag( "truthPdgId"   , -9999., help="Apply pdgId selection to truth particle" )
    icf.addFlag( "truthIsHadron", False, help="Select hadrons" )
    icf.addFlag( "truthIsPion", False, help="Select pions" )
    # Histogram properties
    icf.addFlag( "plotTrackParameters"      , True )
    icf.addFlag( "plotTrackMultiplicities"  , True )
    icf.addFlag( "plotEfficiencies"         , True )
    icf.addFlag( "plotTechnicalEfficiencies", False )
    icf.addFlag( "plotResolutions"          , True )
    icf.addFlag( "plotFakeRates"            , True )
    icf.addFlag( "unlinkedAsFakes"          , True )
    icf.addFlag( "plotDuplicateRates"       , False )
    icf.addFlag( "plotHitsOnTracks"         , True )
    icf.addFlag( "plotHitsOnTracksReference", False )
    icf.addFlag( "plotHitsOnMatchedTracks"  , False )
    icf.addFlag( "plotHitsOnFakeTracks"     , False )
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
                if fname.startswith( "_comment" ): continue
                setattr( flags.PhysVal.IDTPM, 
                        trkAnaName+"."+fname, fvalue )

    if trkAnaNames:
        flags.PhysVal.IDTPM.trkAnaNames = trkAnaNames
    return flags
