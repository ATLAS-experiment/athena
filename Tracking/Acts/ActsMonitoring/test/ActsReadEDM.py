#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def getAllAvailableTracks(flags) -> list[str]:
    from ActsConfig.ActsCollectionsConfig import TrackBackends
    typedCollections = flags.Input.TypedCollections

    trackCollections = []
    for typedCollection in typedCollections:
        [colType, colName] = typedCollection.split('#')

        if colType not in TrackBackends.types:
            continue
        
        prefix = TrackBackends.extractPrefix(collection=colName)
        if f"{prefix}Tracks" not in trackCollections:
            trackCollections.append(f"{prefix}Tracks")

    return trackCollections

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Detector.EnableITkPixel = True
    flags.Detector.EnableITkStrip = True
    flags.DQ.useTrigger = False
    flags.Output.HISTFileName = "ActsMonitoringOutput.root" 

    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = -1
    flags.Exec.OutputLevel = 3
    
    flags.addFlag("readClusters", False)
    flags.addFlag("readSpacePoints", False)
    flags.addFlag("readTracks", False)
    flags.addFlag("tracks", "")
    flags.addFlag("readTrackParticles", False)
    flags.addFlag("trackParticles", "InDetActsTrackParticles")
    flags.addFlag("redoAmbiguity", False)
    flags.fillFromArgs()
    
    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from ActsConfig.ActsCollectionsConfig import ActsPoolReadCfg
    acc.merge(ActsPoolReadCfg(flags))
    
    if flags.readClusters:
        if flags.Detector.EnableITkPixel:
            from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg
            acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripClusterAnalysisAlgCfg
            acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    if flags.readSpacePoints:
        if flags.Detector.EnableITkPixel:             
            from ActsConfig.ActsAnalysisConfig import ActsPixelSpacePointAnalysisAlgCfg
            acc.merge(ActsPixelSpacePointAnalysisAlgCfg(flags))
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripSpacePointAnalysisAlgCfg, ActsStripOverlapSpacePointAnalysisAlgCfg
            acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags))
            acc.merge(ActsStripOverlapSpacePointAnalysisAlgCfg(flags))

    if flags.readTracks:
        from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
        for track in flags.tracks.split(','):
            acc.merge(ActsTrackAnalysisAlgCfg(flags,
                                              name=f"{track}AnalysisAlg",
                                              OutputLevel=2,     
                                              TracksLocation=track))
    if flags.redoAmbiguity:
        from ActsConfig.ActsTrackFindingConfig import ActsMainAmbiguityResolutionAlgCfg
        for track in flags.tracks.split(','):
            acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags,
                                                        name=f"ActsAmbiguityResolutionAlg_{track}",
                                                        TracksLocation=track,
                                                        ResolvedTracksLocation=f"Redone{track}ResolvedTracks"))
    if flags.readTrackParticles:
        allTracks = getAllAvailableTracks(flags)
        from ActsConfig.ActsAnalysisConfig import ActsTrackParticleAnalysisAlgCfg
        for tp in flags.trackParticles.split(','):
            acc.merge(ActsTrackParticleAnalysisAlgCfg(flags,
                                                      name=f"{tp}AnalysisAlg",
                                                      OutputLevel=2,
                                                      TrackParticleLocation=tp,
                                                      ExtraInputs={ ('ActsTrk::TrackContainer', el) for el in allTracks },
                                                      MonGroupName=f"{tp}Analysis"))
            
    acc.printConfig()
    status = acc.run()
    if status.isFailure():
        print("Problem while reading Acts EDM objects from AOD input file ...")
        import sys
        sys.exit(-1)
    
