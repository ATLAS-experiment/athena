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

    # In case perigee expression is Vertex we have a situation where
    # there is a first temporary track particle creation wrt BeamLine
    # followed, after vertex reco, of a second particle creation wrt vertex
    #
    # The final track particle collection will still be the one defined in trackParticleContainerName
    persistifyCollection = True
    particleCollection = trackParticleContainerName
    perigeeExpression = flags.Tracking.perigeeExpression
    if flags.Tracking.perigeeExpression == "Vertex":
        # We do not want to persistify this temporary collection
        persistifyCollection = False
        particleCollection = f"{trackParticleContainerName}Temporary"
        perigeeExpression = "BeamLine"
        
    # Track particles wrt BeamLine
    from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
    acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                              TrackContainers = InputCombinedITkTracks,
                                              TrackParticleContainer = particleCollection,
                                              persistifyCollection = persistifyCollection,
                                              PerigeeExpression = perigeeExpression))
        
    # Vertex reconstruction
    if flags.Tracking.doVertexFinding:
        from InDetConfig.ActsPriVxFinderConfig import primaryVertexFindingCfg
        acc.merge(primaryVertexFindingCfg(flags,
                                          name = "ActsPriVxFinderAlg",
                                          TracksName = particleCollection,
                                          vxCandidatesOutputName = primaryVertices))

    # Track particles wrt Vertex
    #
    # In case perigee expression is Vertex we need to schedule the final
    # track particle creation using the vertex
    # The track collection(s) unchanged, only the final track particle container
    # has a different name
    if flags.Tracking.perigeeExpression == "Vertex":
        assert flags.Tracking.doVertexFinding, \
            f"Requested the computation of track particles wrt but flags.Tracking.doVertexFinding is set to {flags.Tracking.doVertexFinding}"
        print('Requesting to compute the track particle collection wrt the Vertex')
        acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                  TrackContainers = InputCombinedITkTracks,
                                                  TrackParticleContainer = trackParticleContainerName))
        
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

