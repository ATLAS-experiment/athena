#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import BeamType, FlagEnum
from TrkConfig.TrkConfigFlags import PrimaryPassConfig

class RoIStrategy(FlagEnum):
    LeadTracks = 'LeadTracks'
    Random = 'Random'
    File = 'File'
    TruthHS = 'TruthHS'

################################################################
    ## create set of tracking cut flags
################################################################
def createTrackingPassFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("extension", "" ) ### for extension

    icf.addFlag("usePrdAssociationTool", False)
    icf.addFlag("isLowPt", False)
    icf.addFlag("useTIDE_Ambi", lambda pcf: pcf.Tracking.doTIDE_Ambi)
    icf.addFlag("useTRTExtension", lambda pcf: pcf.Tracking.doTRTExtension)
    icf.addFlag("storeSeparateContainer", False)
    icf.addFlag("doAmbiguityProcessorTrackFit", True)

    # --- first set kinematic defaults
    icf.addFlag("minPT", lambda pcf: (pcf.BField.configuredSolenoidFieldScale *
                                      (0.4 * Units.GeV if pcf.Tracking.doLowMu else
                                       0.5 * Units.GeV)))
    icf.addFlag("maxPT", 1000.0 * Units.TeV) # off!
    icf.addFlag("minEta", -1) # off!
    icf.addFlag("maxEta", 2.7)


    # --- cluster cuts
    icf.addFlag("minClusters", lambda pcf:
                3 if (pcf.Detector.EnablePixel and not pcf.Detector.EnableSCT) else
                6 if (pcf.Detector.EnableSCT and not pcf.Detector.EnablePixel) else
                7 if pcf.Tracking.doLowMu else
                8 )

    icf.addFlag("minSiNotShared", 6)
    
    icf.addFlag("maxShared", 1) # cut is now on number of shared modules
    icf.addFlag("minPixel", 0)
    icf.addFlag("maxHoles", lambda pcf: 3 if pcf.Tracking.doLowMu else 2)
    icf.addFlag("maxPixelHoles", lambda pcf: 2 if pcf.Tracking.doLowMu else 1)
    icf.addFlag("maxSctHoles", 2)
    icf.addFlag("maxDoubleHoles", 1)

    icf.addFlag("maxPrimaryImpact", lambda pcf:
                10.0 * Units.mm if (pcf.Tracking.doBLS or pcf.Tracking.doLowMu) else
                5.0 * Units.mm)
    icf.addFlag("maxEMImpact", 50.0 * Units.mm)
    icf.addFlag("maxZImpact", lambda pcf:
                320.0 * Units.mm if pcf.Tracking.doLowMu else
                200.0 * Units.mm)

    # --- this is for the TRT-extension
    icf.addFlag("minTRTonTrk", 9)
    icf.addFlag("minTRTPrecFrac", 0.3)

    # --- general pattern cuts for NewTracking

    # default R cut for SP in SiSpacePointsSeedMaker
    icf.addFlag("radMax", 600.0 * Units.mm)
    icf.addFlag("roadWidth", lambda pcf:   20. if pcf.Tracking.doLowMu else 12.)
    icf.addFlag("nHolesMax", lambda pcf:     3 if pcf.Tracking.doLowMu else 2)
    icf.addFlag("nHolesGapMax", lambda pcf:  3 if pcf.Tracking.doLowMu else 2)
    icf.addFlag("Xi2max", lambda pcf:      15. if pcf.Tracking.doLowMu else 9.)
    icf.addFlag("Xi2maxNoAdd", lambda pcf: 35. if pcf.Tracking.doLowMu else 25.)
    icf.addFlag("nWeightedClustersMin", 6)

    # --- seeding
    icf.addFlag("useSeedFilter", True)
    icf.addFlag("maxTracksPerSharedPRD", 0)  ## is 0 ok for default??
    icf.addFlag("maxdImpactPPSSeeds", lambda pcf: 1.7 if pcf.Tracking.doLowMu else 2.)
    icf.addFlag("maxdImpactSSSSeeds", lambda pcf:
                1000. * Units.mm if pcf.Tracking.doLowMu else
                10. * Units.mm if pcf.Tracking.doBLS else
                5. * Units.mm)
    icf.addFlag("maxSeedsPerSP_Pixels", lambda pcf: 5 if pcf.Tracking.doLowMu else 1)
    icf.addFlag("maxSeedsPerSP_Strips", 5)
    icf.addFlag("keepAllConfirmedPixelSeeds", lambda pcf: not pcf.Tracking.doLowMu)
    icf.addFlag("keepAllConfirmedStripSeeds", False)

    # --- min pt cut for brem
    icf.addFlag("doBremRecoverySi", lambda pcf: pcf.Tracking.doBremRecovery)
    icf.addFlag("minPTBrem", lambda pcf: (
        1. * Units.GeV * pcf.BField.configuredSolenoidFieldScale))

    # --- Z Boundary Seeding
    icf.addFlag("doZBoundary", lambda pcf:
                not (pcf.Beam.Type is BeamType.Cosmics or pcf.Tracking.doLowMu))

    icf.addFlag("usePixel"       		  , lambda pcf : pcf.Detector.EnablePixel )
    icf.addFlag("useTRT"        		  , lambda pcf : pcf.Detector.EnableTRT )
    icf.addFlag("useSCT"        		  , lambda pcf : pcf.Detector.EnableSCT )
    icf.addFlag("usePixelSeeding"        	  , lambda pcf : pcf.Detector.EnablePixel )
    icf.addFlag("useSCTSeeding"        	  	  , lambda pcf : pcf.Detector.EnableSCT )

    # --- Pixel and TRT particle ID during particle creation
    icf.addFlag("RunPixelPID", True)
    icf.addFlag("RunTRTPID", True)

    # --- Flags for detailed information. 
    #     Ignored for Primary Pass (always active); 
    #     Enable for other passes with dedicated output container, if desired.
    icf.addFlag("storeTrackSeeds", False)
    icf.addFlag("storeSiSPSeededTracks", False)

    return icf


### ITk mode ####################
def createITkTrackingPassFlags():

    # Set ITk flags from scratch to avoid relying on InDet flags through lambda functions
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("extension", "" ) ### for extension

    icf.addFlag("useITkPixel"       		  , lambda pcf : pcf.Detector.EnableITkPixel )
    icf.addFlag("useITkStrip"        		  , lambda pcf : pcf.Detector.EnableITkStrip )
    icf.addFlag("useITkPixelSeeding"        	  , True )
    icf.addFlag("useITkStripSeeding"        	  , True )

    icf.addFlag("usePrdAssociationTool"     , False)
    icf.addFlag("storeSeparateContainer"    , False)
    icf.addFlag("doZBoundary"               , True)
    icf.addFlag("doAmbiguityProcessorTrackFit", True)

    icf.addFlag("useTIDE_Ambi", lambda pcf: pcf.Tracking.doTIDE_Ambi)

    # Maximum bin set to 9999 instead of four to prevent out of bounds lookups
    icf.addFlag("etaBins"                   , [-1.0, 2.0, 2.6, 9999.0])
    icf.addFlag("maxEta"                    , 4.0)
    icf.addFlag("minPT"                     , lambda pcf :
                [0.2 * Units.GeV * pcf.BField.configuredSolenoidFieldScale]
                if pcf.Tracking.doLowMu else
                [0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
                 0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
                 0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])

    icf.addFlag("minPTSeed"                 , lambda pcf : (
        pcf.BField.configuredSolenoidFieldScale *
        (0.2 * Units.GeV if pcf.Tracking.doLowMu
         else 0.9 * Units.GeV)))
    icf.addFlag("maxPrimaryImpactSeed"      , 2.0 * Units.mm)
    icf.addFlag("maxZImpactSeed"            , 200.0 * Units.mm)
    icf.addFlag("useSeedFilter"             , True)

    # --- cluster cuts
    icf.addFlag("minClusters"             , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [9, 8, 7])
    icf.addFlag("minSiNotShared"          , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [7, 6, 5])
    icf.addFlag("maxShared"               , [2])
    icf.addFlag("minPixel"                , [1])
    icf.addFlag("maxHoles"                , [2])
    icf.addFlag("maxPixelHoles"           , [2])
    icf.addFlag("maxSctHoles"             , [2])
    icf.addFlag("maxDoubleHoles"          , [1])
    icf.addFlag("maxPrimaryImpact"        , [2.0 * Units.mm, 2.0 * Units.mm, 10.0 * Units.mm])
    icf.addFlag("maxEMImpact"             , [50.0 * Units.mm])
    icf.addFlag("maxZImpact"              , [200.0 * Units.mm])

    # --- general pattern cuts for NewTracking
    icf.addFlag("roadWidth"               , 20.)
    icf.addFlag("nHolesMax"               , icf.maxHoles)
    icf.addFlag("nHolesGapMax"            , icf.maxHoles)

    icf.addFlag("Xi2max"                  , [9.0])
    icf.addFlag("Xi2maxNoAdd"             , [25.0])
    icf.addFlag("nWeightedClustersMin"    , [6])

    # --- seeding
    icf.addFlag("maxdImpactSSSSeeds"      , [20.0 * Units.mm])
    icf.addFlag("radMax"                  , 1100. * Units.mm)

    # --- min pt cut for brem
    icf.addFlag("doBremRecoverySi", lambda pcf: pcf.Tracking.doBremRecovery)
    icf.addFlag("minPTBrem", lambda pcf: (
        [1. * Units.GeV * pcf.BField.configuredSolenoidFieldScale]))

    # -- use of calo information
    icf.addFlag("doCaloSeededBremSi", lambda pcf: pcf.Tracking.doCaloSeededBrem)
    icf.addFlag("doCaloSeededAmbiSi", lambda pcf: pcf.Tracking.doCaloSeededAmbi)

    # --- handle ACTS workflow coexistence
    # Athena components
    icf.addFlag("doAthenaCluster", True)
    icf.addFlag("doAthenaSpacePoint", True)
    icf.addFlag("doAthenaSeed", True)
    icf.addFlag("doAthenaTrack", True)
    icf.addFlag("doAthenaAmbiguityResolution", True)
    # Acts components
    icf.addFlag("doActsCluster", False)
    icf.addFlag("doActsSpacePoint", False)
    icf.addFlag("doActsSeed", False)
    icf.addFlag("doActsTrack", False)
    icf.addFlag("doActsAmbiguityResolution", False)
    # Athena -> Acts EDM converters
    icf.addFlag("doAthenaToActsCluster", False)
    icf.addFlag("doAthenaToActsSpacePoint", False)
    icf.addFlag("doAthenaToActsTrack", False)
    # Acts -> Athena EDM converters
    icf.addFlag("doActsToAthenaCluster", False)
    icf.addFlag("doActsToAthenaSpacePoint", False)
    icf.addFlag("doActsToAthenaSeed", False)
    icf.addFlag("doActsToAthenaTrack", False)
    icf.addFlag("doActsToAthenaResolvedTrack", False)

    # --- flags for GNN tracking
    icf.addFlag("doGNNTrack", False)

    # ---flag for FPGA tracking
    icf.addFlag("doFPGASpacePoint", False)
    icf.addFlag("doFPGATrack", False)

    # --- Flags for detailed information. 
    #     Ignored for Primary Pass (always active); 
    #     Enable for other passes with dedicated output container, if desired.
    icf.addFlag("storeTrackSeeds", False)
    icf.addFlag("storeSiSPSeededTracks", False)

    # --- flags for ACTS tracking
    icf.addFlag("isSecondaryPass", False)
    return icf



## Heavyion mode #######################
def createITkHeavyIonTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension        = "HeavyIon"
    icf.maxPrimaryImpact = [2.0 * Units.mm]
    icf.minPT            = lambda pcf : (
        [0.4 *Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.minPTSeed        = lambda pcf : (
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters      = [6]
    icf.minSiNotShared   = [6]
    icf.Xi2max           = [6.]
    icf.Xi2maxNoAdd      = [9.]
    icf.maxPixelHoles    = [1] 
    icf.maxSctHoles      = [1] 
    icf.maxDoubleHoles   = [0] 
    icf.doBremRecoverySi = False
    return icf


def createITkFastTrackingPassFlags():

    icf = createITkTrackingPassFlags()

    icf.minPT                 = lambda pcf : (
        [1.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
         0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
         0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.maxZImpact            = [150.0 * Units.mm]
    icf.minPixel              = [3]
    icf.maxHoles              = [1]
    icf.nHolesMax             = icf.maxHoles
    icf.nHolesGapMax          = [1]
    icf.minPTSeed             = lambda pcf: (
        1.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxZImpactSeed        = 150.0 * Units.mm
    icf.useITkStripSeeding    = False

    return icf

### ITk with FTF standalone mode ####
def createITkFTFPassFlags():

    icf = createITkFastTrackingPassFlags()

    icf.addFlag("doHitDV"            , False)
    icf.addFlag("doDisappearingTrk"  , False)
    icf.addFlag("useTrigTrackFollowing", False)
    icf.addFlag("useTrigRoadPredictor", False)
    icf.addFlag("useTracklets", False)
    icf.useSeedFilter         = False
    icf.minPT                 = lambda pcf : (
        [0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
         0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
         0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.minPTSeed             = lambda pcf : (
        0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)

    return icf

def createITkFTFLargeD0PassFlags():
    icf = createITkLargeD0FastTrackingPassFlags()
    icf.addFlag("doHitDV"            , False)
    icf.addFlag("doDisappearingTrk"  , False)
    icf.addFlag("useTrigTrackFollowing", False)
    icf.addFlag("useTrigRoadPredictor", False)
    icf.addFlag("useTracklets", False)
    icf.useSeedFilter         = False
    icf.minPT              = lambda pcf : (
        [1.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])

    icf.maxPrimaryImpact   = [400.0 * Units.mm]
    icf.maxPrimaryImpactSeed = 400.0 * Units.mm
    icf.maxdImpactSSSSeeds = [400.0 * Units.mm]
    

    return icf


### ITk LRT mode ####################
def createITkLargeD0TrackingPassFlags():

    icf = createITkTrackingPassFlags()
    icf.extension             = "LargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container

    icf.minPT              = lambda pcf : (
        [1000 * Units.MeV * pcf.BField.configuredSolenoidFieldScale])
    icf.maxEta             = 4.0
    icf.etaBins            = [-1.0, 4.0]
    icf.maxPrimaryImpact   = [300 * Units.mm]
    icf.maxZImpact         = [500 * Units.mm]
    icf.minClusters        = [8]
    icf.minSiNotShared     = [6]
    icf.maxShared          = [2]
    icf.minPixel           = [0]
    icf.maxHoles           = [1]
    icf.maxPixelHoles      = [1]
    icf.maxSctHoles        = [1]
    icf.maxDoubleHoles     = [0]

    icf.maxZImpactSeed     = 500.0 * Units.mm
    icf.maxPrimaryImpactSeed = 300.0 * Units.mm
    icf.minPTSeed          = lambda pcf : (
        1000 * Units.MeV * pcf.BField.configuredSolenoidFieldScale)

    icf.radMax             = 1100. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles
    icf.roadWidth          = 5

    # --- seeding
    icf.useITkPixelSeeding       = False
    icf.maxdImpactSSSSeeds       = [300.0 * Units.mm]

    icf.doBremRecoverySi = False

    icf.Xi2max                  = [9.0]
    icf.Xi2maxNoAdd             = [25.0]
    icf.nWeightedClustersMin    = [6]

    return icf

def createITkLargeD0FastTrackingPassFlags():

    icf = createITkLargeD0TrackingPassFlags()

    icf.useITkPixelSeeding = False
    icf.useITkStripSeeding = True

    icf.maxEta             = 2.4
    icf.etaBins            = [-1.0, 2.4]
    icf.minPT              = lambda pcf : (
        [5.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.minPTSeed          = lambda pcf : (
        5.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.nWeightedClustersMin = [8]
    icf.maxPrimaryImpact   = [150 * Units.mm]
    icf.maxPrimaryImpactSeed = 150. * Units.mm
    icf.maxdImpactSSSSeeds = [150.0 * Units.mm]
    icf.maxZImpact         = [200 * Units.mm]
    icf.maxZImpactSeed     = 200. * Units.mm
    icf.radMax             = 400. * Units.mm

    return icf

### ITk LowPt mode ####################
def createITkLowPtTrackingPassFlags():

    icf = createITkTrackingPassFlags()
    icf.extension          = "LowPt"
    icf.minPT              = lambda pcf : (
        [0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.minPTSeed          = lambda pcf : (
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.doBremRecoverySi   = False

    return icf

### HighPileUP mode ####################
def createHighPileupTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "HighPileup"
    icf.minPT                   = lambda pcf : (
        0.900 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters             = 9
    icf.maxPixelHoles           = 0
    icf.doBremRecoverySi        = False

    return icf

## MinBias mode ########################
def createMinBiasTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension                 = "MinBias"
    icf.minPT                     = lambda pcf: (
        0.1 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxPrimaryImpact          = 10. * Units.mm
    icf.maxZImpact                = 250. * Units.mm
    icf.minClusters               = 5
    icf.maxdImpactSSSSeeds        = 20.0 * Units.mm # apply cut on SSS seeds
    icf.roadWidth                 = 20.
    icf.doBremRecoverySi          = False
    icf.maxSeedsPerSP_Pixels      = 5
    icf.keepAllConfirmedPixelSeeds = False

    return icf

## UPC  mode ########################
def createUPCTrackingPassFlags():
    icf = createMinBiasTrackingPassFlags()
    icf.extension                 = "UPC"
    # --- min pt cut for brem
    icf.minPTBrem                 = lambda pcf: (
        0.75 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    # MinBias turns off Brem Recovery, turn it on here
    icf.doBremRecoverySi = lambda pcf: pcf.Tracking.doBremRecovery
    return icf

## HIP  mode ########################
def createHIPTrackingPassFlags():
    icf = createMinBiasTrackingPassFlags()
    icf.extension                 = "HIP"
    return icf


## LowPtRoI mode ########################
def createLowPtRoITrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "LowPtRoI"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = True
    icf.maxPT              = lambda pcf: (
        0.850 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minPT              = lambda pcf: (
        0.050 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters        = 5
    icf.minSiNotShared     = 4
    icf.maxShared          = 1   # cut is now on number of shared modules
    icf.minPixel           = 2
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 2
    icf.maxDoubleHoles     = 1
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles # not as tight as 2*maxDoubleHoles
    icf.doBremRecoverySi   = False
    # Add custom flags valid for this pass only
    icf.addFlag("z0WindowRoI", 30.0) # mm
    icf.addFlag("doRandomSpot", False)
    icf.addFlag("RoIStrategy", RoIStrategy.LeadTracks)
    icf.addFlag("inputLowPtRoIfile","")

    return icf


## R3LargeD0 mode ########################
def createR3LargeD0TrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "R3LargeD0"
    icf.usePrdAssociationTool = True
    icf.usePixelSeeding    = False
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container
    icf.maxPT              = lambda pcf : (
        1.0 * Units.TeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minPT              = lambda pcf : (
        1.0 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxEta             = 3
    icf.maxPrimaryImpact   = 300.0 * Units.mm
    icf.maxEMImpact        = 300 * Units.mm
    icf.maxZImpact         = 500 * Units.mm    
    icf.minClusters        = 8                  
    icf.minSiNotShared     = 6                 
    icf.maxShared          = 2   # cut is now on number of shared modules
    icf.minPixel           = 0
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 1  
    icf.maxDoubleHoles     = 0  
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = 1
    icf.maxTracksPerSharedPRD   = 2
    icf.Xi2max                  = 9.0  
    icf.Xi2maxNoAdd             = 25.0 
    icf.roadWidth               = 5. 
    icf.nWeightedClustersMin    = 8   
    icf.maxdImpactSSSSeeds      = 300.0
    icf.doZBoundary             = True
    icf.keepAllConfirmedStripSeeds = True
    icf.maxSeedsPerSP_Strips = 1
    icf.doBremRecoverySi = False

    icf.RunPixelPID        = False
    icf.RunTRTPID          = False

    return icf

## LowPtLargeD0 mode ########################
def createLowPtLargeD0TrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension          = "LowPtLargeD0"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = lambda pcf : pcf.Tracking.storeSeparateLargeD0Container
    icf.maxPT              = lambda pcf: (
        1.0 * Units.TeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minPT              = lambda pcf: (
        100 * Units.MeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxEta             = 5
    icf.maxPrimaryImpact   = 300.0 * Units.mm
    icf.maxZImpact         = 1500.0 * Units.mm
    icf.minClusters        = 5
    icf.minSiNotShared     = 5
    icf.maxShared          = 2   # cut is now on number of shared modules
    icf.minPixel           = 0
    icf.maxHoles           = 2
    icf.maxPixelHoles      = 1
    icf.maxSctHoles        = 2
    icf.maxDoubleHoles     = 1
    icf.radMax             = 600. * Units.mm
    icf.nHolesMax          = icf.maxHoles
    icf.nHolesGapMax       = icf.maxHoles
    icf.maxTracksPerSharedPRD = 2
    icf.doBremRecoverySi = False

    icf.RunPixelPID        = False
    icf.RunTRTPID          = False

    return icf

## LowPt mode ########################
def createLowPtTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "LowPt"
    icf.usePrdAssociationTool = True
    icf.isLowPt          = True
    icf.maxPT            = lambda pcf: (
        pcf.BField.configuredSolenoidFieldScale *
        (1e6 if pcf.Tracking.doMinBias else
         pcf.Tracking.MainPass.minPT + 0.3) * Units.GeV)
    icf.minPT            = lambda pcf: (
        0.05 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters      = 5
    icf.minSiNotShared   = 4
    icf.maxShared        = 1   # cut is now on number of shared modules
    icf.minPixel         = 2   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
    icf.maxHoles         = 2
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 2
    icf.maxDoubleHoles   = 1
    icf.radMax           = 600. * Units.mm
    icf.nHolesMax        = icf.maxHoles
    icf.nHolesGapMax     = icf.maxHoles # not as tight as 2*maxDoubleHoles
    icf.maxPrimaryImpact = lambda pcf: (
        100. * Units.mm if pcf.Tracking.doMinBias else 10. * Units.mm)
    icf.doBremRecoverySi = False
    
    return icf

## ITkConversion mode ########################
def createITkConversionTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension               = "Conversion"
    icf.usePrdAssociationTool   = True

    icf.etaBins                 = [-1.0,4.0]
    icf.minPT                   = lambda pcf: (
        [0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale])
    icf.maxPrimaryImpact        = [10.0 * Units.mm]
    icf.maxZImpact              = [150.0 * Units.mm]
    icf.minClusters             = [6]
    icf.minSiNotShared          = [6]
    icf.maxShared               = [0]
    icf.minPixel                = [0]
    icf.maxHoles                = [0]
    icf.maxPixelHoles           = [1]
    icf.maxSctHoles             = [2]
    icf.maxDoubleHoles          = [1]

    icf.nHolesMax               = icf.maxHoles
    icf.nHolesGapMax            = icf.maxHoles
    icf.nWeightedClustersMin    = [6]
    icf.maxdImpactSSSSeeds      = [20.0 * Units.mm]
    icf.radMax                  = 1100. * Units.mm
    icf.doZBoundary             = False

    icf.Xi2max                  = [9.0]
    icf.Xi2maxNoAdd             = [25.0]
    icf.doBremRecoverySi        = True

    return icf

## VeryLowPt mode ########################
def createVeryLowPtTrackingPassFlags():
    icf = createTrackingPassFlags() #TODO consider using createLowPtTrackingPassFlags as a base here
    icf.extension        = "VeryLowPt"
    icf.usePrdAssociationTool = True
    icf.isLowPt          = True
    icf.useTRTExtension  = False
    icf.maxPT            = lambda pcf : (
        pcf.BField.configuredSolenoidFieldScale *
        (1e6 if pcf.Tracking.doMinBias else 
         pcf.Tracking.MainPass.minPT + 0.3) * Units.GeV)
    icf.minPT            = lambda pcf : (
        0.050 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters      = 3
    icf.minSiNotShared   = 3
    icf.maxShared        = 1   # cut is now on number of shared modules
    icf.minPixel         = 3   # At least one pixel hit for low-pt (assoc. seeded on pixels!)
    icf.maxHoles         = 1
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 1
    icf.maxDoubleHoles   = 0
    icf.nHolesMax        = 1
    icf.nHolesGapMax     = 1 # not as tight as 2*maxDoubleHoles
    icf.radMax           = 600. * Units.mm # restrivt to pixels
    icf.doBremRecoverySi        = False

    return icf

## ForwardTracks mode ########################
def createForwardTracksTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Forward"
    icf.usePrdAssociationTool = True
    icf.useTIDE_Ambi     = False
    icf.useTRTExtension  = False
    icf.storeSeparateContainer = True
    icf.minEta           = 2.4 # restrict to minimal eta
    icf.maxEta           = 2.7
    icf.minPT            = lambda pcf: (
        2 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters      = 3
    icf.minSiNotShared   = 3
    icf.maxShared        = 1
    icf.minPixel         = 3
    icf.maxHoles         = 1
    icf.maxPixelHoles    = 1
    icf.maxSctHoles      = 1
    icf.maxDoubleHoles   = 0
    icf.nHolesMax        = icf.maxHoles
    icf.nHolesGapMax     = icf.maxHoles
    icf.radMax           = 600. * Units.mm
    icf.useTRT           = False # no TRT for forward tracks
    icf.doBremRecoverySi = False
    icf.doZBoundary      = False

    icf.RunPixelPID      = False
    icf.RunTRTPID        = False

    return icf

## BeamGas mode ########################
def createBeamGasTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "BeamGas"
    icf.usePrdAssociationTool = True
    icf.minPT            = lambda pcf: (
        0.5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxPrimaryImpact = 300. * Units.mm
    icf.maxZImpact       = 2000. * Units.mm
    icf.minClusters      = 6
    icf.maxHoles         = 3
    icf.maxPixelHoles    = 3
    icf.maxSctHoles      = 3
    icf.maxDoubleHoles   = 1
    icf.nHolesMax        = 3
    icf.nHolesGapMax     = 3 # not as tight as 2*maxDoubleHoles
    icf.doBremRecoverySi = False

    return icf

## VtxLumi mode ########################
def createVtxLumiTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "VtxLumi"
    icf.minPT                   = lambda pcf: (
        0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters             = 7
    icf.maxPixelHoles           = 1
    icf.radMax                  = 600. * Units.mm
    icf.nHolesMax               = 2
    icf.nHolesGapMax            = 1
    icf.useTRT                  = False
    icf.doBremRecoverySi        = False

    return icf

## VtxLumiHeavyIon mode ################
def createVtxLumiHeavyIonTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "VtxLumiHeavyIon"
    icf.minPT                   = lambda pcf: (
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxPrimaryImpact        = 10. * Units.mm
    icf.maxZImpact              = 250.0 * Units.mm
    icf.minClusters             = 7
    icf.maxPixelHoles           = 1
    icf.radMax                  = 600. * Units.mm
    icf.nHolesMax               = 2
    icf.nHolesGapMax            = 1
    icf.maxdImpactSSSSeeds      = 20.
    icf.roadWidth               = 20.
    icf.keepAllConfirmedPixelSeeds = False
    icf.maxSeedsPerSP_Pixels    = 5
    icf.useTRT                  = False
    icf.doBremRecoverySi        = False

    return icf

## VtxBeamSpot mode ########################
def createVtxBeamSpotTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "VtxBeamSpot"
    icf.minPT                   = lambda pcf: (
        0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters             = 9
    icf.maxPixelHoles           = 0
    icf.radMax                  = 320. * Units.mm
    icf.nHolesMax               = 2
    icf.nHolesGapMax            = 1
    icf.useTRT                  = False
    icf.doBremRecoverySi        = False

    return icf

## Cosmics mode ########################
def createCosmicsTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Cosmics"
    icf.minPT            = lambda pcf: (
        0.5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.maxPrimaryImpact = 1000. * Units.mm
    icf.maxZImpact       = 10000. * Units.mm
    icf.minClusters      = 4
    icf.minSiNotShared   = 4
    icf.maxHoles         = 3
    icf.maxPixelHoles    = 3
    icf.maxSctHoles      = 3
    icf.maxDoubleHoles   = 1
    icf.minTRTonTrk      = 15
    icf.roadWidth        = 60.
    icf.Xi2max           = 60.
    icf.Xi2maxNoAdd      = 100.
    icf.nWeightedClustersMin = 8
    icf.nHolesMax        = 3
    icf.nHolesGapMax     = 3 # not as tight as 2*maxDoubleHoles
    icf.maxdImpactSSSSeeds = 20. * Units.mm
    icf.doBremRecoverySi = False
    icf.doZBoundary      = False
    icf.maxSeedsPerSP_Pixels = 5
    icf.keepAllConfirmedPixelSeeds = False

    return icf

## Heavyion mode #######################
def createHeavyIonTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "HeavyIon"
    icf.maxPrimaryImpact = 10. * Units.mm
    icf.maxZImpact       = 200. * Units.mm
    icf.minClusters      = 9
    icf.minSiNotShared   = 7
    icf.maxShared        = 2 # was 1, cut is now on number of shared modules

    icf.nHolesMax        = 0
    icf.nHolesGapMax     = 0
    icf.Xi2max           = 6.
    icf.Xi2maxNoAdd      = 10.

    icf.minPT              = lambda pcf: (
        0.5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
        
    icf.maxdImpactSSSSeeds =  20. * Units.mm
    icf.maxdImpactPPSSeeds = 1.7
    
    icf.maxHoles = 2
    icf.maxPixelHoles = 1
    icf.maxSctHoles = 1
    icf.maxDoubleHoles   = 0    
    icf.Xi2max           = 9.
    icf.Xi2maxNoAdd      = 25.
    icf.radMax           = 600. * Units.mm # restrict to pixels + first SCT layer
    icf.roadWidth        = 20.
    icf.useTRT           = False
    icf.doBremRecoverySi = False
    icf.doZBoundary      = False
    icf.maxSeedsPerSP_Pixels = 5
    icf.keepAllConfirmedPixelSeeds = False

    return icf

### Pixel mode ###############################################
def createPixelTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Pixel"
    icf.isLowPt          = lambda pcf : pcf.Tracking.doMinBias

    def _minPt( pcf ):
        if pcf.Beam.Type is BeamType.Cosmics:
            return 0.5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        if pcf.Tracking.PrimaryPassConfig is PrimaryPassConfig.UPC:
            return 0.05 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        if pcf.Tracking.PrimaryPassConfig is PrimaryPassConfig.HIP:
            return 0.05 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        if pcf.Tracking.doMinBias:
            return 0.05 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        return 0.1 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
    
    icf.minPT            = _minPt
    icf.minClusters      = 3

    def _pick( default, hion, cosmics):
        def _internal( pcf ):
            if pcf.Tracking.PrimaryPassConfig is PrimaryPassConfig.HeavyIon:
                return hion
            if pcf.Beam.Type is BeamType.Cosmics:
                return cosmics
            return default
        return _internal
    
    icf.maxHoles         = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.maxPixelHoles    = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.maxSctHoles      = 0
    icf.maxDoubleHoles   = 0
    icf.minSiNotShared   = 3
    icf.maxShared        = 0
    icf.nHolesMax        = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.nHolesGapMax     = _pick( default = 1, hion = 0, cosmics = 3 )
    icf.useSCT           = False
    icf.useSCTSeeding    = False
    icf.useTRT           = False
    icf.maxPrimaryImpact = lambda pcf: (
        1000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics else
        10. * Units.mm if pcf.Tracking.doUPC else
        5. * Units.mm)
    icf.roadWidth        = lambda pcf: (
        60.0 if pcf.Beam.Type is BeamType.Cosmics else
        12.0)
    icf.maxZImpact       = lambda pcf: (
        10000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics else
        320. * Units.mm if pcf.Tracking.doLowMu else
        250. * Units.mm if pcf.Tracking.doMinBias else
        200. * Units.mm)
    icf.Xi2max           = lambda pcf: (
        60. if pcf.Beam.Type is BeamType.Cosmics else
        15. if pcf.Tracking.doLowMu else
        9.)
    icf.Xi2maxNoAdd      = lambda pcf: (
        100.0  if pcf.Beam.Type is BeamType.Cosmics else
        35. if pcf.Tracking.doLowMu else
        25.)
    icf.nWeightedClustersMin = 6
    icf.doBremRecoverySi        = False

    icf.RunPixelPID      = lambda pcf: (
        pcf.Tracking.PrimaryPassConfig is PrimaryPassConfig.UPC)
    icf.RunTRTPID        = False
    return icf

########## Disappearing mode ######################
def createDisappearingTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "Disappearing"
    icf.usePrdAssociationTool = True
    icf.storeSeparateContainer = True
    icf.minPT            = lambda pcf: (
        5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.minClusters      = 4
    icf.maxHoles         = 0
    icf.maxPixelHoles    = 0
    icf.maxSctHoles      = 0
    icf.maxDoubleHoles   = 0
    icf.minSiNotShared   = 3
    icf.maxShared        = 0
    icf.nHolesMax        = 0
    icf.nHolesGapMax     = 0
    icf.useSCT           = True
    icf.useTRT           = True
    icf.useSCTSeeding    = False
    icf.maxEta           = 2.2
    icf.doBremRecoverySi = False
    def MainPassFlags(pcf):
        return pcf.Tracking.__getattr__(pcf.Tracking.PrimaryPassConfig.value+'Pass')
    icf.maxPrimaryImpact = lambda pcf: MainPassFlags(pcf).maxPrimaryImpact
    icf.maxZImpact = lambda pcf: MainPassFlags(pcf).maxZImpact
    icf.roadWidth = lambda pcf: MainPassFlags(pcf).roadWidth
    return icf

########## SCT mode ######################
def createSCTTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension        = "SCT"
    icf.minClusters      = 7
    icf.maxDoubleHoles   = 1
    icf.minSiNotShared   = 5
    icf.usePixel         = False
    icf.usePixelSeeding  = False
    icf.useTRT           = False

    def _minpt( pcf ):
        if pcf.Beam.Type is BeamType.Cosmics:
            return 0.5 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        if pcf.Tracking.doMinBias:
            return 0.1 * Units.GeV * pcf.BField.configuredSolenoidFieldScale
        return 0.1 * Units.GeV * pcf.BField.configuredSolenoidFieldScale

    icf.minPT            = _minpt

    icf.maxPrimaryImpact = lambda pcf: (
        1000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics else
        10. * Units.mm if pcf.Tracking.doLowMu else
        5. * Units.mm)
    icf.maxZImpact       = lambda pcf: (
        10000. * Units.mm if pcf.Beam.Type is BeamType.Cosmics else
        320. * Units.mm if pcf.Tracking.doLowMu else
        200.0 * Units.mm)

    icf.maxHoles         = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics else 2
    icf.nHolesMax        = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics else 2
    icf.nHolesGapMax     = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics else 2
    icf.maxPixelHoles    = lambda pcf: 0 if pcf.Beam.Type is BeamType.Cosmics else 0
    icf.maxSctHoles      = lambda pcf: 3 if pcf.Beam.Type is BeamType.Cosmics else 2
    icf.maxShared        = 0
    icf.roadWidth        = lambda pcf: (
        60. if pcf.Beam.Type is BeamType.Cosmics else
        20. if pcf.Tracking.doLowMu else
        12.)
    icf.Xi2max           = lambda pcf: (
        60. if pcf.Beam.Type is BeamType.Cosmics else
        15. if pcf.Tracking.doLowMu else
        9.)
    icf.Xi2maxNoAdd      = lambda pcf: (
        100.0 if pcf.Beam.Type is BeamType.Cosmics else
        35. if pcf.Tracking.doLowMu else
        25.)
    icf.nWeightedClustersMin = lambda pcf: 4 if pcf.Beam.Type is BeamType.Cosmics else 6
    icf.minClusters      = lambda pcf: (
        4 if pcf.Beam.Type is BeamType.Cosmics else
        7 if pcf.Tracking.doLowMu else
        8)
    icf.minSiNotShared   = lambda pcf: 4 if pcf.Beam.Type is BeamType.Cosmics else 5
    icf.doBremRecoverySi        = False
    
    icf.RunPixelPID      = False
    icf.RunTRTPID        = False
    return icf

########## TRT subdetector tracklet cuts  ##########
def createTRTTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension               = "TRT"
    icf.useTIDE_Ambi            = False
    icf.usePrdAssociationTool   = True
    icf.minPT                   = lambda pcf: (
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    icf.doBremRecoverySi        = False

    icf.RunPixelPID             = False
    icf.RunTRTPID               = False
    return icf


########## TRT standalone tracklet cuts  ##########
def createTRTStandaloneTrackingPassFlags():
    icf = createTrackingPassFlags()
    icf.extension              = "TRTStandalone"
    icf.useTIDE_Ambi           = False
    icf.usePrdAssociationTool  = True
    icf.doBremRecoverySi        = False

    return icf

#####################################################################

def printPrimaryConfig(flags):
    print()
    print("************************************************************************")

    print("******************** Tracking reconstruction Config ********************")
    print("                     Primary Config is",flags.Tracking[flags.Tracking.PrimaryPassConfig.value+"Pass"].extension)
    flags.dump(pattern="Tracking."+flags.Tracking.PrimaryPassConfig.value+"Pass.*", evaluate=True)
    print("************************************************************************")
    return


def printActiveConfig(flags):
    print()
    print("************************************************************************")

    print("******************** Tracking reconstruction Config ********************")
    print("                     Active Config is",flags.Tracking.ActiveConfig.extension)
    flags.dump(pattern="Tracking.ActiveConfig.*", evaluate=True)
    print("************************************************************************")
    return


#####################################################################
#####################################################################

if __name__ == "__main__":

  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  from AthenaConfiguration.TestDefaults import defaultTestFiles
  flags.Input.Files=defaultTestFiles.RAW_RUN2
  
  from AthenaCommon.Logging import logging
  l = logging.getLogger('TrackingPassFlags')
  from AthenaCommon.Constants import INFO
  l.setLevel(INFO)

  flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")

  assert flags.Tracking.ActiveConfig.maxPrimaryImpact == 5.0 * Units.mm, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.maxPrimaryImpact)
  assert flags.Tracking.HeavyIonPass.maxPrimaryImpact == 10.0 * Units.mm, "wrong cut value {} ".format(flags.Tracking.HeavyIonPass.maxPrimaryImpact)
  flags.Tracking.doBLS = True
  assert flags.Tracking.ActiveConfig.maxPrimaryImpact == 10.0 * Units.mm, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.maxPrimaryImpact)

  assert flags.Tracking.ActiveConfig.minPT == 0.5 * Units.GeV, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.minPT)
  flags.BField.configuredSolenoidFieldScale = 0.1
  assert flags.Tracking.ActiveConfig.minPT == 0.05 * Units.GeV, "wrong cut value {} ".format(flags.Tracking.ActiveConfig.minPT)      
        
  l.info("flags.Tracking.ActiveConfig.minPT %f", flags.Tracking.ActiveConfig.minPT * 1.0)
  l.info("type(flags.Tracking.ActiveConfig.minPT) " + str(type(flags.Tracking.ActiveConfig.minPT)))




