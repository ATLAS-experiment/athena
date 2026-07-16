# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 

from TrkConfig.TrackingPassFlags import createTrackingPassFlags, createITkTrackingPassFlags, createITkFastTrackingPassFlags, createITkConversionTrackingPassFlags, createITkHeavyIonTrackingPassFlags, createITkLargeD0TrackingPassFlags, createITkLowPtTrackingPassFlags
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

def setActsDefaultTunings(icf):
    # ACTS specifc config flags
    icf.addFlag("isSecondaryPass", False)
    icf.addFlag("isLargeD0", False)
    icf.addFlag("autoReverseSearch", False)
    # Extension used to name the persistified track particle container
    # (InDet{extension}TrackParticles) when storeSeparateContainer is
    # requested. If empty, the pass extension is used.
    icf.addFlag("storedTrackParticlesExtension", "")
    
    # Custom values for config flags
    icf.Xi2max = [25]
    icf.Xi2maxNoAdd = [25]


# Main ACTS Tracking pass    
def createActsLegacyTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsLegacy"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    setActsDefaultTunings(icf)
    return icf

# Main ACTS Tracking pass with Fast Tracking configuration
def createActsTrackingPassFlags():
    icf = createITkFastTrackingPassFlags()
    icf.extension = "Acts"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    setActsDefaultTunings(icf)
    
    # Override acts default values
    icf.Xi2max = [50]
    icf.Xi2maxNoAdd = [100]
    return icf

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
    # Store the output track particles in InDetLargeD0TrackParticles
    # (instead of InDetActsLargeRadiusTrackParticles) so that downstream
    # LRT clients can rely on the same container name as in Run 3
    icf.storedTrackParticlesExtension = "LargeD0"

    # Override acts default values
    icf.Xi2max = [75]
    icf.Xi2maxNoAdd = [100]

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

def createActsValidateClustersTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsValidateClusters"
    deactivateAthenaComponents(icf)
    icf.doActsCluster = True
    icf.doActsToAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaAmbiguityResolution = True
    setActsDefaultTunings(icf)
    return icf

def createActsValidateLargeRadiusStandaloneTrackingPassFlags():
    icf = createActsLargeRadiusTrackingPassFlags()
    icf.extension = "ActsValidateLargeRadiusStandalone"
    icf.isSecondaryPass = False
    icf.isLargeD0 = True
    # Validation pass keeps the default InDet{extension}TrackParticles name
    icf.storedTrackParticlesExtension = ""
    return icf

def createActsValidateTracksTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = lambda pcf : "ActsValidateTracks" if not pcf.Acts.doAmbiguityResolution else "ActsValidateResolvedTracks"
    deactivateAthenaComponents(icf)
    # sequence is still a work in progress
    # Requires Athena cluster and cluster EDM converter 
    # for adding decoration to cluster objects
    # It produces Athena TrackCollection EDM
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

    # Deactivate CTIDE processor fit
    icf.doAmbiguityProcessorTrackFit = False
    setActsDefaultTunings(icf)
    return icf

def createActsValidateAmbiguityResolutionTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsValidateAmbiguityResolution"
    deactivateAthenaComponents(icf)
    # The sequence will schedule Athena algorithms from clustering to 
    # track reconstruction, but not the ambi. resolution
    # We convert tracks, run the acts ambi. resolution and convert 
    # resolved tracks back to Athena EDM
    icf.doAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaToActsTrack = True
    icf.doActsAmbiguityResolution = True
    icf.doActsToAthenaResolvedTrack = True
    setActsDefaultTunings(icf)
    return icf

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
    return icf

# Main Inner Detector ACTS Tracking pass
def createACTSInnerDetectorTrackingPassFlags():
    # flags for ACTS based InnerDetector silicon tracking
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