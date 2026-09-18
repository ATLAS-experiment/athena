# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration 

from TrkConfig.TrackingPassFlags import createITkConversionTrackingPassFlags, createITkHeavyIonTrackingPassFlags, createITkLargeD0TrackingPassFlags, createITkLowPtTrackingPassFlags
from ActsConfig.ActsConfigFlags import SeedingStrategy
import AthenaCommon.SystemOfUnits as Units

def deactivateAthenaComponents(icf):
    icf.doAthenaCluster = False
    icf.doAthenaSpacePoint = False
    icf.doAthenaSeed = False
    icf.doAthenaTrack = False
    icf.doAthenaAmbiguityResolution = False
    icf.doActsCluster = False
    icf.doActsSpacePoint = False
    icf.doActsSeed = False
    icf.doActsTrack = False
    icf.doActsAmbiguityResolution = False

def activateActsComponents(icf):
    icf.doActsCluster = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    # Ambiguity resolution can follow if ActsTrack is 
    # enabled. Ambi. can be activated/deactivated with 
    # the flag: Acts.doAmbiguityResolution
    icf.doActsAmbiguityResolution = lambda pcf: pcf.Acts.doAmbiguityResolution

# Main ACTS Tracking pass with Fast Tracking configuration
def createActsTrackingPassFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("extension", "Acts" )

    icf.addFlag("useITkPixel", lambda pcf : pcf.Detector.EnableITkPixel )
    icf.addFlag("useITkStrip", lambda pcf : pcf.Detector.EnableITkStrip )
    icf.addFlag("useITkPixelSeeding", True )
    icf.addFlag("useITkStripSeeding", False )

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
    icf.addFlag("maxPrimaryImpactSeed"      , 5.0 * Units.mm)
    icf.addFlag("maxZImpactSeed"            , 150.0 * Units.mm)
    icf.addFlag("useSeedFilter"             , True)
    icf.addFlag("useHoughVertexFilter"      , False) # experimental, keep False

    # --- cluster cuts
    icf.addFlag("minClusters"             , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [9, 8, 7])
    icf.addFlag("minSiNotShared"          , lambda pcf :
                [6, 5, 4] if pcf.Tracking.doLowMu else [7, 6, 5])
    icf.addFlag("maxShared"               , [2])
    icf.addFlag("minPixel"                , [3])
    icf.addFlag("maxHoles"                , [1])
    icf.addFlag("maxPixelHoles"           , [2])
    icf.addFlag("maxSctHoles"             , [2])
    icf.addFlag("maxDoubleHoles"          , [1])
    icf.addFlag("maxPrimaryImpact"        , [5.0 * Units.mm, 5.0 * Units.mm, 10.0 * Units.mm])
    icf.addFlag("maxEMImpact"             , [50.0 * Units.mm])
    icf.addFlag("maxZImpact"              , [150.0 * Units.mm])

    # --- general pattern cuts for NewTracking
    icf.addFlag("roadWidth"               , 20.)
    icf.addFlag("nHolesMax"               , icf.maxHoles)
    icf.addFlag("nHolesGapMax"            , icf.maxHoles)

    icf.addFlag("Xi2max"                  , [50.])
    icf.addFlag("Xi2maxNoAdd"             , [100.])
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

    # ACTS specifc config flags
    icf.addFlag("isSecondaryPass", False)
    icf.addFlag("isLargeD0", False)
    icf.addFlag("autoReverseSearch", False)
    icf.addFlag("PixelSeedingStrategy", SeedingStrategy.Gbts, type=SeedingStrategy)
    icf.addFlag("StripSeedingStrategy", SeedingStrategy.GridTriplet, type=SeedingStrategy)
    # Extension used to name the persistified track particle container
    # (InDet{extension}TrackParticles) when storeSeparateContainer is
    # requested. If empty, the pass extension is used.
    icf.addFlag("storedTrackParticlesExtension", "")

    # --- handle ACTS workflow coexistence
    # Athena components
    icf.addFlag("doAthenaCluster", False)
    icf.addFlag("doAthenaSpacePoint", False)
    icf.addFlag("doAthenaSeed", False)
    icf.addFlag("doAthenaTrack", False)
    icf.addFlag("doAthenaAmbiguityResolution", False)
    # Acts components
    icf.addFlag("doActsCluster", True)
    icf.addFlag("doActsSpacePoint", True)
    icf.addFlag("doActsSeed", True)
    icf.addFlag("doActsTrack", True)
    icf.addFlag("doActsAmbiguityResolution", True)

    # Athena -> Acts EDM converters
    icf.addFlag("doAthenaToActsCluster", False)
    icf.addFlag("doAthenaToActsSpacePoint", False)
    icf.addFlag("doAthenaToActsTrack", False)
    # Acts -> Athena EDM converters
    icf.addFlag("doActsToAthenaCluster", False)
    icf.addFlag("doActsToAthenaSpacePoint", False)
    icf.addFlag("doActsToAthenaTrack", False)
    icf.addFlag("doActsToAthenaResolvedTrack", False)

    # --- flags for GNN tracking
    icf.addFlag("doGNNTrack", False)

    # ---flag for FPGA tracking
    icf.addFlag("doFPGASpacePoint", False)
    icf.addFlag("doFPGACluster", False)
    icf.addFlag("doFPGASeed", False)
    icf.addFlag("doFPGATrackSim", False)

    # --- Flags for detailed information. 
    #     Ignored for Primary Pass (always active); 
    #     Enable for other passes with dedicated output container, if desired.
    icf.addFlag("storeTrackSeeds", False)
    icf.addFlag("storeSiSPSeededTracks", False)

    return icf


def createActsLegacyTrackingPassFlags():
    icf = createActsTrackingPassFlags()
    icf.extension = "ActsLegacy"
    icf.maxZImpact = [200. * Units.mm]
    icf.maxZImpactSeed = 200. * Units.mm
    icf.Xi2max = [25.]
    icf.Xi2maxNoAdd = [25.]
    icf.PixelSeedingStrategy = SeedingStrategy.GridTriplet
    icf.useITkStripSeeding = True
    icf.minPixel = [1]
    icf.maxHoles = [2]
    return icf



def setActsDefaultTunings(icf):
    # Custom values for config flags
    icf.Xi2max = [25]
    icf.Xi2maxNoAdd = [25]


# Main ACTS Tracking pass with Heavy Ion configuration
# For the current time this is still an hybrid configuration
# We need to apply additional changes to the JO to support
# cases where the ambiguity solver is not scheduled
def createActsHeavyIonTrackingPassFlags():
    icf = createITkHeavyIonTrackingPassFlags()
    icf.extension = "ActsHeavyIon"
    deactivateAthenaComponents(icf)
    icf.doAthenaCluster = True
    icf.doAthenaToActsCluster = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    # If we do not want acts ambi resolution, first do the track convertion
    # and then the Athena ambi
    icf.doActsToAthenaTrack = lambda pcf : not pcf.Acts.doAmbiguityResolution
    icf.doAthenaAmbiguityResolution = lambda pcf : not pcf.Acts.doAmbiguityResolution
    # If we want acts ambi, first do the ambi and then convert the tracks
    # without Athena ambi
    icf.doActsAmbiguityResolution = lambda pcf : pcf.Acts.doAmbiguityResolution
    icf.doActsToAthenaResolvedTrack = lambda pcf : pcf.Acts.doAmbiguityResolution

    setActsDefaultTunings(icf)
    # Deactivate CTIDE processor fit
    icf.doAmbiguityProcessorTrackFit = False    
    return icf

# Secondary ACTS Tracking pass for Large Radius Tracking
def createActsLargeRadiusTrackingPassFlags():
    icf = createITkLargeD0TrackingPassFlags()
    icf.extension = "ActsLargeRadius"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    setActsDefaultTunings(icf)
    icf.useITkPixelSeeding = False
    # Store the output track particles in InDetLargeD0TrackParticles
    # (instead of InDetActsLargeRadiusTrackParticles) so that downstream
    # LRT clients can rely on the same container name as in Run 3
    icf.storedTrackParticlesExtension = "LargeD0"

    # Override acts default values
    icf.Xi2max = [25]
    icf.Xi2maxNoAdd = [50]

    # Mark as secondary pass
    icf.isSecondaryPass = True
    # Store sepate container for LRT
    # In Athena this is handled by the Tracking.storeSeparateLargeD0Container flag
    icf.storeSeparateContainer = True
    icf.isLargeD0 = True
    icf.autoReverseSearch = True
    return icf

# Secondary ACTS Tracking pass for Conversion tracking
def createActsConversionTrackingPassFlags():
    icf = createITkConversionTrackingPassFlags()
    icf.extension = "ActsConversion"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    setActsDefaultTunings(icf)
    icf.useITkPixelSeeding = False
    # Mark as secondary pass
    icf.isSecondaryPass = True
    # Conversion pass is usually merged with main pass
    icf.storeSeparateContainer = False
    return icf

# Secondary ACTS Tracking pass for Low pT tracking
def createActsLowPtTrackingPassFlags():
    icf = createITkLowPtTrackingPassFlags()
    icf.extension = "ActsLowPt"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    setActsDefaultTunings(icf)
    # Mark as secondary pass
    icf.isSecondaryPass = True
    # For the time being we do not store sepate containers for this pass (to be revised)
    # In Athena this is handled by the Tracking.storeSeparateLargeD0Container flag
    icf.storeSeparateContainer = False
    return icf


# Validation chains

def createEFValidateF100TrackingPassFlags():
    icf = createActsTrackingPassFlags()
    icf.extension = "ActsValidateF100"
    icf.doActsCluster = False
    icf.doFPGACluster = True
    icf.doFPGATrackSim = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    
    # Override acts default values
    icf.Xi2max = [50]
    icf.Xi2maxNoAdd = [100]
    icf.useITkStripSeeding = False
    return icf

def createEFValidateF150TrackingPassFlags():
    icf = createActsTrackingPassFlags()
    icf.extension = "ActsValidateF150"
    icf.doActsCluster = False
    icf.doFPGACluster = True
    icf.doFPGASeed = True
    icf.doFPGATrackSim = True
    icf.doActsSpacePoint = False
    icf.doActsSeed = False
    icf.doActsTrack = True

    # Override acts default values
    icf.Xi2max = [50]
    icf.Xi2maxNoAdd = [100]
    icf.PixelSeedingStrategy = SeedingStrategy.F150
    icf.useITkStripSeeding = False
    return icf

# Main Inner Detector ACTS Tracking pass
def createACTSInnerDetectorTrackingPassFlags():
    # flags for ACTS based InnerDetector silicon tracking
    from TrkConfig.TrackingPassFlags import createTrackingPassFlags
    icf = createTrackingPassFlags()
    setActsDefaultTunings(icf)
    icf.extension               = ""
    icf.Xi2max = 25.0
    icf.Xi2maxNoAdd = 25.0
    # ACTS components
    icf.addFlag("doActsCluster", True)
    icf.addFlag("doActsSpacePoint", True)
    icf.addFlag("doActsSeed", True)
    icf.addFlag("doActsTrack", True)
    icf.addFlag("doActsAmbiguityResolution", True)

    # Maximum bin set to 9999 instead of four to prevent out of bounds lookups
    icf.addFlag("etaBins"                   , [-1.0, 3.0, 9999.0])
    icf.addFlag("maxPrimaryImpactList"      , [5.0 * Units.mm, 5.0 * Units.mm, 25.0 * Units.mm])
    return icf
