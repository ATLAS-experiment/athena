# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator



def ITkActsTrackRecoCfg(flags) -> ComponentAccumulator:
    # Main Job Option for ACTS Track Reconstruction with ITk
    print("Scheduling the ACTS Job Option for ITk Track Reconstruction")
    if flags.Tracking.doITkFastTracking:
        print("- Configuration requested: Fast Tracking")
    acc = ComponentAccumulator()

    # Pre-Processing
    # Retrieve all the tracking passes
    from InDetConfig.ITkActsHelpers import extractTrackingPasses
    scheduledTrackingPasses = extractTrackingPasses(flags)
    # Keep track of previous pass (used for PRD mapping)
    previousExtension = None

    # Track Collections to be merged for main track particle collection
    # This is the groups of tracks generated in tracking passes that do not store
    # tracks in separate containers
    InputCombinedITkTracks = []
    
    # Container names
    trackParticleContainerName = "InDetTrackParticles"
    primaryVertices = "PrimaryVertices"

    # Reconstruction
    from InDetConfig.ITkActsHelpers import isPrimaryPass
    for currentFlags in scheduledTrackingPasses:
        # Printing configuration
        print(f"---- Preparing scheduling of algorithms for tracking pass: {currentFlags.Tracking.ActiveConfig.extension}")
        print(f"---- - Is primary pass: {isPrimaryPass(currentFlags)}")
        from TrkConfig.TrackingPassFlags import printActiveConfig
        printActiveConfig(currentFlags)

        # Data Preparation
        # This includes Region-of-Interest creation, Cluster and Space Point formation
        from InDetConfig.ITkActsDataPreparationConfig import ITkActsDataPreparationCfg
        acc.merge(ITkActsDataPreparationCfg(currentFlags,
                                            previousExtension = previousExtension))
        
        # Track Reconstruction        
        # This includes Seeding, Track Finding (CKF) and Ambiguity Resolution
        from InDetConfig.ITkActsPatternRecognitionConfig import ITkActsTrackReconstructionCfg
        acc.merge(ITkActsTrackReconstructionCfg(currentFlags,
                                                previousExtension = previousExtension))

        # Update variables
        previousExtension = currentFlags.Tracking.ActiveConfig.extension
        if not currentFlags.Tracking.ActiveConfig.storeSeparateContainer or isPrimaryPass(currentFlags):
            acts_tracks = f"{currentFlags.Tracking.ActiveConfig.extension}Tracks" if not currentFlags.Acts.doAmbiguityResolution else f"{currentFlags.Tracking.ActiveConfig.extension}ResolvedTracks"
            InputCombinedITkTracks.append(acts_tracks)


    # Track particle creation
    print(f"Creating track particle collection '{trackParticleContainerName}' from combination of following track collection:")
    for trackCollection in InputCombinedITkTracks:
        print(f'- {trackCollection}')

    from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
    acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                              TrackContainers = InputCombinedITkTracks,
                                              TrackParticleContainer = trackParticleContainerName))
        
    # Vertex reconstruction
    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        acc.merge(primaryVertexFindingCfg(flags,
                                          name = "ActsPriVxFinderAlg",
                                          TracksName = trackParticleContainerName,
                                          vxCandidatesOutputName = primaryVertices))
        
    # Post-Processing
    print('Starting Post-Processing')
    for currentFlags in scheduledTrackingPasses:
        # Particle persistification for tracking pass
        from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticlePersistificationCfg
        acc.merge(ITkActsTrackParticlePersistificationCfg(currentFlags))

    ## ACTS Specific write PRDInfo
    if flags.Tracking.writeExtendedSi_PRDInfo:
        # Add the truth origin to the truth particles
        # Despite the name if the Cfg function, this handles:
        # - Pixel detector
        # - Strip detector
        from InDetConfig.InDetPrepRawDataToxAODConfig import ITkActsPixelPrepDataToxAODCfg
        acc.merge(ITkActsPixelPrepDataToxAODCfg(flags))

    acc.printConfig(withDetails = False, summariseProps = False)
    return acc

