# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def isPrimaryPass(flags) -> bool:
    return f"{flags.Tracking.ITkPrimaryPassConfig.value}Pass" not in flags.Tracking

def primaryPassUsesActs(flags) -> bool:
    from TrkConfig.TrkConfigFlags import ITkPrimaryPassConfig
    return flags.Tracking.ITkPrimaryPassConfig in [ITkPrimaryPassConfig.Acts, \
                                                   ITkPrimaryPassConfig.ActsFast, \
                                                   ITkPrimaryPassConfig.ActsHeavyIon]


def extractTrackingPasses(flags) -> list:
    # Function for extracting the requested tracking passes that need to be scheduled
    trackingPasses = []

    # Check there is only one chain
    # for the time being we still technically allow for a list, but we should move to a single value eventually
    assert len(flags.Tracking.recoChain) == 1, \
        f"Conflicting reco configuration: Tracking.recoChain should have only one element but we found {flags.Tracking.recoChain}"
    
    # Quick check about fast tracking configuration
    from TrkConfig.TrkConfigFlags import ITkPrimaryPassConfig
    if flags.Tracking.ITkPrimaryPassConfig is ITkPrimaryPassConfig.ActsFast:
        assert flags.Tracking.doITkFastTracking, \
            f"Main pass is set to Fast Tracking but Tracking.doITkFastTracking is set to {flags.Tracking.doITkFastTracking}"
    else:
        assert not flags.Tracking.doITkFastTracking, \
            f"Main pass is NOT set to Fast Tracking but Tracking.doITkFastTracking is set to {flags.Tracking.doITkFastTracking}"
        
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
        assert flags.Detector.EnableCalo, \
            "Problem in the job configuration: required reconstruction of photon conversion tracks but Calorimeter Detector is not enabled"
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

