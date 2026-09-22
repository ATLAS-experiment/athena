def enableTrackMon(flags, cfg, suffix, verbose=False, alg_names=['ActsTrackFindingAlg','ActsLargeRadiusTrackFindingAlg','ActsHGTDTrackExtensionAlg']) :
    from AthenaConfiguration.ComponentFactory import CompFactory

    for alg_name in alg_names :
        # for alg_name in ['ActsTrackFindingAlg'] :
        ckf=cfg.getEventAlgo(alg_name)
        cluster_to_truth_maps=[]
        cluster_to_truth_maps += [ 'ITkPixelClustersToTruthParticles' ] if flags.Detector.EnableITkPixel else []
        cluster_to_truth_maps += [ 'ITkStripClustersToTruthParticles' ] if flags.Detector.EnableITkStrip else []
        cluster_to_truth_maps += [ 'HgtdClustersToTruthParticles' ]     if flags.Detector.EnableHGTD else []
        extension=alg_name.replace("Acts","").replace("FindingAlg","")
        measurements=[]
        if alg_name.find('LargeRadius')>=0 :
            measurements += [ 'ITkLargeRadiusPixelClusters' ] if flags.Detector.EnableITkPixel else []
            measurements += [ 'ITkLargeRadiusStripClusters' ] if flags.Detector.EnableITkStrip else []

        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs_outputlevel={}
        if verbose :
            ckf.OutputLevel=1
            kwargs_outputlevel.setdefault("OutputLevel",1)

        ckf.TrackFindingMonitor = CompFactory.ActsTrk.TrackMonitorTool(
            NTupleFileName=f'TrackMonitorNT{extension}{suffix}_AllTruth.root',
            ClustersToTruthAssociationMap=cluster_to_truth_maps,
            MeasurementsForMasking=measurements,
            MinHitsForTruthTrajectory=3,
            MaxEnergyLossElasticDecay=1e12,
            WriteTruthMode=2,
            **kwargs_outputlevel
        )
