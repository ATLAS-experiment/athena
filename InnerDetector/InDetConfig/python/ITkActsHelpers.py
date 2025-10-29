# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def isPrimaryPass(flags) -> bool:
    if not flags.hasCategory("Tracking.ActiveConfig"):
        return False
    # Support for non ACTS passes, that do not respect the convention
    # This comes from Athena legacy passes
    if flags.Tracking.ActiveConfig.extension in ["", "HeavyIon"]:
        return True
    # For ACTS the convention is respected: ITk{extension} + Pass
    return f"ITk{flags.Tracking.ActiveConfig.extension}" == flags.Tracking.ITkPrimaryPassConfig.value

def isFastPrimaryPass(flags) -> bool:
    if flags.hasCategory("Tracking.ActiveConfig") and flags.Tracking.ActiveConfig.extension in ["ActsValidateF100", "ActsValidateF150"] and flags.Tracking.doITkFastTracking:
        return True
    return flags.Tracking.doITkFastTracking and isPrimaryPass(flags)

def isValidationPass(flags) -> bool:
    return "Validate" in flags.Tracking.ActiveConfig.extension

def isProductionPass(flags) -> bool:
    return not isValidationPass(flags)

def primaryPassUsesActs(flags) -> bool:
    from TrkConfig.TrkConfigFlags import ITkPrimaryPassConfig
    return flags.Tracking.ITkPrimaryPassConfig in [ITkPrimaryPassConfig.Acts, \
                                                   ITkPrimaryPassConfig.ActsLegacy, \
                                                   ITkPrimaryPassConfig.ActsHeavyIon]

def primaryPassExtension(flags) -> str:
    # we rely on the fact that flags.Tracking.ITkPrimaryPassConfig.value is
    # equal to ITk{extension}
    return flags.Tracking.ITkPrimaryPassConfig.value.replace("ITk", "")

def extractTrackingPasses(flags) -> list:
    # Function for extracting the requested tracking passes that need to be scheduled
    trackingPasses = []

    # Check there is only one chain
    # for the time being we still technically allow for a list, but we should move to a single value eventually
    if len(flags.Tracking.recoChain) != 1:
        raise ValueError(f"Conflicting reco configuration: Tracking.recoChain should have only one element but we found {flags.Tracking.recoChain}")
    
    # Quick check about fast tracking configuration
    from TrkConfig.TrkConfigFlags import ITkPrimaryPassConfig
    if flags.Tracking.ITkPrimaryPassConfig is ITkPrimaryPassConfig.Acts:
        if not flags.Tracking.doITkFastTracking:
            raise ValueError(f"Main pass is set to Acts Fast Tracking but Tracking.doITkFastTracking is set to {flags.Tracking.doITkFastTracking}")
    else:
        if flags.Tracking.doITkFastTracking:
            raise ValueError(f"Main pass is NOT set to Fast Tracking but Tracking.doITkFastTracking is set to {flags.Tracking.doITkFastTracking}")

    # Check the ambiguity resolution strategy
    if flags.Acts.doAmbiguityResolution:
        from ActsConfig.ActsConfigFlags import AmbiguitySolverMode
        # If ambiguity resolution is requested, it means we want to schedule the ambiguity resolution algorithm
        # this means that we must have AmbiguitySolverMode.OUTSIDE_TF
        if flags.Acts.AmbiguitySolverMode is not AmbiguitySolverMode.OUTSIDE_TF:
            raise ValueError(f"Conflicting reco configuration: Acts.doAmbiguityResolution has been requested and this will schedule the ACTS ambiguity solver algorithm, yet the ambiguity mode (set to {flags.Acts.AmbiguitySolverMode}) is not compatible with this.")


    # Primary pass
    trackingPasses += [flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.ITkPrimaryPassConfig.value}Pass")]

    # Conversion pass
    if flags.Acts.doITkConversion:
        # Check that we can schedule the conversion
        if not flags.Detector.EnableCalo:
            raise ValueError("Problem in the job configuration: required reconstruction of photon conversion tracks but Calorimeter Detector is not enabled")
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsConversionPass")]

    # Large Radius pass
    if flags.Acts.doLargeRadius:
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLargeRadiusPass")]

        
    # Low pT pass
    if flags.Acts.doLowPt:
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLowPtPass")]
                
    print("List of scheduled passes:")
    for trackingPass in trackingPasses:
        print(f'- {trackingPass.Tracking.ActiveConfig.extension}')
    
    # Check if we found a primary pass (and only one)
    nPrimaryPasses = 0
    for current_flags in trackingPasses:
        if isPrimaryPass(current_flags):
            nPrimaryPasses += 1
    if nPrimaryPasses != 1:
        raise ValueError(f"Problem in the job configuration: exactly one primary pass is required for a proper configuration, but we found {nPrimaryPasses} instead!")
    
    return trackingPasses

def getListOfGeneratedTrackParticles(flags) -> list[str]:
    generateTrackCollections = ["InDetTrackParticles"]

    # loop on tracking passes
    scheduledTrackingPasses: list = extractTrackingPasses(flags)
    for currentFlags in scheduledTrackingPasses:
        # Add the seed tracks
        if currentFlags.Tracking.ActiveConfig.storeTrackSeeds:
            # pixel seeds
            generatePixelSegments = currentFlags.Detector.EnableITkPixel
            generateStripSegments = currentFlags.Detector.EnableITkStrip
            
            # For conversion pass we do not process pixels
            if currentFlags.Tracking.ActiveConfig.extension in ["ActsConversion", "ActsLargeRadius"]:
                generatePixelSegments = False
                # For main pass disable strips if fast tracking configuration
            elif isFastPrimaryPass(currentFlags):
                generateStripSegments = False

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
