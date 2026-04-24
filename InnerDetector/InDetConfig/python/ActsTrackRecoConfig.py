# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def isPrimaryPass(flags) -> bool:
    if not flags.hasCategory("Tracking.ActiveConfig"):
        return False
    # Support for non ACTS passes, that do not respect the convention
    # This comes from Athena legacy passes
    if flags.Tracking.ActiveConfig.extension in ["", "HeavyIon"]:
        return True
    
    return f"{flags.Tracking.ActiveConfig.extension}" == flags.Tracking.PrimaryPassConfig.value

def extractTrackingPasses(flags) -> list:
    # Function for extracting the requested tracking passes that need to be scheduled
    trackingPasses = []

    # Primary pass
    trackingPasses += [flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass")]
    
    # Only have primary pass for the moment
    print("List of scheduled passes:")
    for trackingPass in trackingPasses:
        print(f'- {trackingPass.Tracking.ActiveConfig.extension}')

    return trackingPasses

def getListOfGeneratedTrackParticles(flags) -> list[str]:
    generateTrackCollections = ["InDetTrackParticles"]

    # loop on tracking passes
    scheduledTrackingPasses: list = extractTrackingPasses(flags)
    for currentFlags in scheduledTrackingPasses:
        # Add the seed tracks
        if currentFlags.Tracking.ActiveConfig.storeTrackSeeds:
            # pixel seeds
            generatePixelSegments = currentFlags.Detector.EnablePixel
            generateStripSegments = currentFlags.Detector.EnableStrip

            if generatePixelSegments:
                generateTrackCollections += [f'SiSPSeedSegments{currentFlags.Tracking.ActiveConfig.extension}PixelTrackParticles']
            if generateStripSegments:
                generateTrackCollections += [f'SiSPSeedSegments{currentFlags.Tracking.ActiveConfig.extension}StripTrackParticles']
            if generatePixelSegments and generateStripSegments:
                generateTrackCollections += [f'SiSPSeedSegments{currentFlags.Tracking.ActiveConfig.extension}TrackParticles']
            
            # Add CKF tracks
            if currentFlags.Tracking.ActiveConfig.storeSiSPSeededTracks:
                generateTrackCollections += [f'SiSPSeededTracks{currentFlags.Tracking.ActiveConfig.extension}TrackParticles']
            
            # Add tracks after ambi
            # this is necessary only if ambiguity resolution is run and we
            # store track particles in a separate container w.r.t InDetTrackParticles
            if currentFlags.Acts.doAmbiguityResolution and currentFlags.Tracking.ActiveConfig.storeSeparateContainer:
                generateTrackCollections += [f'InDet{currentFlags.Tracking.ActiveConfig.extension}TrackParticles']

    print('Here is the list of generated track particle collections:')
    for collection in generateTrackCollections:
        print(f'- {collection}')
        
    return generateTrackCollections

def ActsTrackRecoCfg(flags) -> ComponentAccumulator:
    # Main Job Option for ACTS Track Reconstruction with Inner Detector
    print("Scheduling the ACTS Job Option for Inner Detector Track Reconstruction")

    acc = ComponentAccumulator()
    
    # Pre-Processing
    # Retrieve all the tracking passes
    scheduledTrackingPasses = extractTrackingPasses(flags)
    # Keep track of previous pass (used for PRD mapping)
    previousExtension = None

    # Track Collections to be merged for main track particle collection
    # This is the groups of tracks generated in tracking passes that do not store
    # tracks in separate containers
    InputCombinedTracks = []

    # Container names
    trackParticleContainerName = "InDetTrackParticles"
    primaryVertices = "PrimaryVertices"

    # Reconstruction
    for currentFlags in scheduledTrackingPasses:
        # Printing configuration
        print(f"---- Preparing scheduling of algorithms for tracking pass: {currentFlags.Tracking.ActiveConfig.extension}")
        print(f"---- - Is primary pass: {isPrimaryPass(currentFlags)}")
        from TrkConfig.TrackingPassFlags import printActiveConfig
        printActiveConfig(currentFlags)

        # Data Preparation
        # This includes Region-of-Interest creation, Cluster and Space Point formation
        from InDetConfig.ActsDataPreparationConfig import ActsDataPreparationCfg
        acc.merge(ActsDataPreparationCfg(currentFlags,
                                            previousExtension = previousExtension))
     
        # Track Reconstruction        
        # This includes Seeding, Track Finding (CKF) and Ambiguity Resolution
        from InDetConfig.ActsPatternRecognitionConfig import ActsTrackReconstructionCfg
        acc.merge(ActsTrackReconstructionCfg(currentFlags,
                                                previousExtension = previousExtension))
  
        # Update variables
        previousExtension = currentFlags.Tracking.ActiveConfig.extension
        if not currentFlags.Tracking.ActiveConfig.storeSeparateContainer or isPrimaryPass(currentFlags):
            acts_tracks = f"{currentFlags.Tracking.ActiveConfig.extension}Tracks" if not currentFlags.Acts.doAmbiguityResolution else f"{currentFlags.Tracking.ActiveConfig.extension}ResolvedTracks"
            InputCombinedTracks.append(acts_tracks)


    # Track particle creation
    print(f"Creating track particle collection '{trackParticleContainerName}' from combination of following track collection:")
    for trackCollection in InputCombinedTracks:
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
                                              TrackContainers = InputCombinedTracks,
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
                                                  TrackContainers = InputCombinedTracks,
                                                  TrackParticleContainer = trackParticleContainerName))
     
    # Post-Processing
    print('Starting Post-Processing')


    ## ACTS Specific write PRDInfo
    # This functionality is not supported in case cluster formation
    # produces clusters in multiple tracking passes
    if flags.Tracking.writeExtendedSi_PRDInfo:        
        # Get all the track particle collections being generated
        # this covers the main tracking collection InDetTrackParticles
        # as well as the converted seeds, tracks from CKF and all those
        # track collection that do not get merged.
        # This operation is necessary only if we desire to only persistify
        # on-track PRD info, since we need to get all the measurements used by
        # all tracks we want to persistify
        generatedTrackParticleCollections = ["InDetTrackParticles"]
        if flags.Tracking.PRDInfo.KeepOnlyOnTrackMeasurements:
            generatedTrackParticleCollections = getListOfGeneratedTrackParticles(flags)
               
        # Add the truth origin to the truth particles
        # This handles:
        # - Pixel detector
        # - Strip detector        
        from InDetConfig.InDetPrepRawDataToxAODConfig import ActsPrepDataToxAODCfg
        acc.merge(ActsPrepDataToxAODCfg(flags,
                                           TrackParticles = generatedTrackParticleCollections))

        # Create MSOS on final InDetTrackParticles collection
        from ActsConfig.ActsObjectDecorationConfig import ActsInDetTrackStateOnSurfaceDecoratorAlgCfg
        acc.merge(ActsInDetTrackStateOnSurfaceDecoratorAlgCfg(flags,
                                                         name=f"Acts{trackParticleContainerName}StateOnSurfaceDecoratorAlg",
                                                         TrackParticles=trackParticleContainerName))
 
    # Run on the specific tracking passes
    for currentFlags in scheduledTrackingPasses:
        # Particle persistification for tracking pass
        from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticlePersistificationCfg
        acc.merge(ITkActsTrackParticlePersistificationCfg(currentFlags))

        # Create MSOS for the intermediate track particle collections
        # this may be the CKF and/or the ambi tracks and can only happen if
        # - storeSiSPSeededTracks for this tracking pass is requested
        # - storeSeparateContainer for this tracking pass is requested
        if flags.Tracking.writeExtendedSi_PRDInfo:
            from ActsConfig.ActsObjectDecorationConfig import ActsInDetTrackStateOnSurfaceDecoratorAlgCfg
            # CKF tracks are called: SiSPSeededTracks{currentFlags.Tracking.ActiveConfig.extension}TrackParticles
            if currentFlags.Tracking.ActiveConfig.storeSiSPSeededTracks:
                TrackParticleCollectionForMsos = f'SiSPSeededTracks{currentFlags.Tracking.ActiveConfig.extension}TrackParticles'
                acc.merge(ActsInDetTrackStateOnSurfaceDecoratorAlgCfg(currentFlags,
                                                                 name=f"{TrackParticleCollectionForMsos}StateOnSurfaceDecoratorAlg",
                                                                 TrackParticles=TrackParticleCollectionForMsos,
                                                                 PixelMSOSs=f"SiSPSeeded{currentFlags.Tracking.ActiveConfig.extension}PixelMSOSs",
                                                                 StripMSOSs=f"SiSPSeeded{currentFlags.Tracking.ActiveConfig.extension}StripMSOSs"))
                
            if currentFlags.Tracking.ActiveConfig.storeSeparateContainer:
                # track collection can be the CKF or the ambi depending
                # on the presence of the ambiguity resolution algorithm
                # but the track particle collection remains the same
                # name: InDet{currentFlags.Tracking.ActiveConfig.extension}TrackParticles
                TrackParticleCollectionForMsos = f'InDet{currentFlags.Tracking.ActiveConfig.extension}TrackParticles'
                acc.merge(ActsInDetTrackStateOnSurfaceDecoratorAlgCfg(currentFlags,
                                                                 name=f"{TrackParticleCollectionForMsos}StateOnSurfaceDecoratorAlg",
                                                                 TrackParticles=TrackParticleCollectionForMsos,
                                                                 PixelMSOSs=f"{currentFlags.Tracking.ActiveConfig.extension}PixelMSOSs",
                                                                 StripMSOSs=f"{currentFlags.Tracking.ActiveConfig.extension}StripMSOSs"))
    # Debug output
    # from ActsConfig.ActsGeometryConfig import ActsWriteTrackingGeometryTransformsAlgCfg
    # acc.merge(ActsWriteTrackingGeometryTransformsAlgCfg(flags,WriteFullTransform = False, OutputName = "transforms.csv"))
    # from AthenaCommon.Constants import DEBUG, ALL, VERBOSE     
    # acc.foreach_component("*/TrackFindingAlg").OutputLevel = DEBUG
    acc.printConfig(withDetails = True, summariseProps = True)
    return acc

