# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 

from TrkConfig.TrackingPassFlags import createITkTrackingPassFlags, createITkFastTrackingPassFlags, createITkConversionTrackingPassFlags, createITkHeavyIonTrackingPassFlags, createITkLargeD0TrackingPassFlags, createITkLowPtTrackingPassFlags

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

    
# Main ACTS Tracking pass    
def createActsLegacyTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsLegacy"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    return icf

# Main ACTS Tracking pass with Fast Tracking configuration
def createActsTrackingPassFlags():
    icf = createITkFastTrackingPassFlags()
    icf.extension = "Acts"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
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

    # Other specific flags
    icf.minPTSeed = 0.4
    # Deactivate CTIDE processor fit
    icf.doAmbiguityProcessorTrackFit = False    
    return icf

# Secondary ACTS Tracking pass for Large Radius Tracking
def createActsLargeRadiusTrackingPassFlags():
    icf = createITkLargeD0TrackingPassFlags()
    icf.extension = "ActsLargeRadius"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
    # Mark as secondary pass 
    icf.isSecondaryPass = True
    # For the time being we do not store sepate containers for LRT (to be revised)
    # In Athena this is handled by the Tracking.storeSeparateLargeD0Container flag
    icf.storeSeparateContainer = False
    return icf

# Secondary ACTS Tracking pass for Conversion tracking
def createActsConversionTrackingPassFlags():
    icf = createITkConversionTrackingPassFlags()
    icf.extension = "ActsConversion"
    deactivateAthenaComponents(icf)
    activateActsComponents(icf)
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
    return icf

def createActsValidateSpacePointsTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsValidateSpacePoints"
    deactivateAthenaComponents(icf)
    icf.doAthenaCluster = True
    icf.doAthenaToActsCluster = True
    icf.doActsSpacePoint = True
    # we should schedule here the Acts -> Athena SP converter, but that is not available yet  
    # so we go for the seeding convertion (i.e. ActsTrk::SiSpacePointSeedMaker) 
    icf.doActsToAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaAmbiguityResolution = True
    return icf

def createActsValidateSeedsTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsValidateSeeds"
    deactivateAthenaComponents(icf)
    icf.doAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaToActsSpacePoint = True
    icf.doActsToAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaAmbiguityResolution = True
    return icf

def createActsValidateConversionSeedsTrackingPassFlags():
    icf = createActsConversionTrackingPassFlags()
    icf.extension = "ActsValidateConversionSeeds"
    deactivateAthenaComponents(icf)
    icf.doAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaToActsSpacePoint = True
    icf.doActsToAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaAmbiguityResolution = True
    icf.isSecondaryPass = False
    return icf

def createActsValidateLargeRadiusSeedsTrackingPassFlags():
    icf = createActsLargeRadiusTrackingPassFlags()
    icf.extension = "ActsValidateLargeRadiusSeeds"
    deactivateAthenaComponents(icf)
    icf.doAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaToActsSpacePoint = True
    icf.doActsToAthenaSeed = True
    icf.doAthenaTrack = True
    icf.doAthenaAmbiguityResolution = True
    icf.isSecondaryPass = False
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
    return icf