# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 

from TrkConfig.TrackingPassFlags import createITkTrackingPassFlags, createITkConversionTrackingPassFlags

def deactivateAthenaComponents(icf):
    icf.doAthenaCluster = False
    icf.doAthenaSpacePoint = False
    icf.doAthenaSeed = False
    icf.doAthenaTrack = False
    icf.doAthenaAmbiguityResolution = False

def createActsTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "Acts"
    deactivateAthenaComponents(icf)
    icf.doActsCluster = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    # Ambiguity resolution can follow if ActsTrack is 
    # enabled. Ambi. can be activated/deactivated with 
    # the flag: Acts.doAmbiguityResolution
    icf.doActsAmbiguityResolution = lambda pcf: pcf.Acts.doAmbiguityResolution

    return icf

def createActsConversionTrackingPassFlags():
    icf = createITkConversionTrackingPassFlags()
    icf.extension = "ActsConversion"
    deactivateAthenaComponents(icf)
    icf.doActsCluster = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    # Ambiguity resolution can follow if ActsTrack is 
    # enabled. Ambi. can be activated/deactivated with 
    # the flag: Acts.doAmbiguityResolution
    icf.doActsAmbiguityResolution = lambda pcf: pcf.Acts.doAmbiguityResolution
    return icf
    
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

def createActsBenchmarkSpotTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "ActsBenchmarkSpot"
    deactivateAthenaComponents(icf)
    # Very not-standard configuration
    icf.doAthenaCluster = True
    icf.doActsCluster = True
    icf.doAthenaToActsCluster = True
    icf.doActsSpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True
    icf.doActsAmbiguityResolution = True
    icf.doActsToAthenaResolvedTrack = True
    return icf


    
