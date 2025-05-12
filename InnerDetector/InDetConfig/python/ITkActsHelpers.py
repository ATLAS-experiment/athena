# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def isPrimaryPass(flags) -> bool:
    return f"{flags.Tracking.ITkPrimaryPassConfig.value}Pass" not in flags.Tracking

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
        
    # Primary pass
    trackingPasses += [flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.ITkPrimaryPassConfig.value}Pass")]

    # Large Radius pass
    if flags.Acts.doLargeRadius:
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLargeRadiusPass")]

    # Conversion pass
    if flags.Acts.doITkConversion:
        # Check that we can schedule the conversion
        if not flags.Detector.EnableCalo:
            raise ValueError("Problem in the job configuration: required reconstruction of photon conversion tracks but Calorimeter Detector is not enabled")
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsConversionPass")]
        
    # Low pT pass
    if flags.Acts.doLowPt:
        trackingPasses += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLowPtPass")]
                
    print("List of scheduled passes:")
    for trackingPass in trackingPasses:
        print(f'- {trackingPass.Tracking.ActiveConfig.extension}')
        
    return trackingPasses

