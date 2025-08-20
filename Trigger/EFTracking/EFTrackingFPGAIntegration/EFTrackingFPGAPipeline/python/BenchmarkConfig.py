# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def BenchmarkCfg(flags, name = 'BenckmarkAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('PixelL2GKernelName','l2g_pixel_tool')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('EDMPrepKernelName', 'EDMPrep')
    kwarg.setdefault('InputPixelClusterKey', 'ITkPixelClusters')
    kwarg.setdefault('InputStripClusterKey', 'ITkStripClusters')
    kwarg.setdefault('runPassThrough', flags.FPGADataPrep.RunPassThrough)
    kwarg.setdefault('doEmulation', flags.FPGADataPrep.DoEmulation)
    

    # Set up Cluster maker tool
    from EFTrackingFPGAPipeline.DataPrepConfig import xAODClusterMakerCfg
    clusterMakerTool = acc.popToolsAndMerge(xAODClusterMakerCfg(flags))
    kwarg.setdefault('xAODClusterMaker', clusterMakerTool)
    
    # Set up TestVectorTool
    from EFTrackingFPGAUtility.FPGADataFormatter import FPGATestVectorToolCfg
    testVectorTool = acc.popToolsAndMerge(FPGATestVectorToolCfg(flags))
    kwarg.setdefault('TestVectorTool', testVectorTool)

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.BenchmarkAlg(**kwarg))

    return acc

def FPGAClusterSortingCfg(flags):
    acc = ComponentAccumulator()
    from FPGAClusterSorting.FPGAClusterSortingConfig import FPGAClusterSortingAlgCfg
    ClusterSorting = FPGAClusterSortingAlgCfg(flags)
    
    acc.merge(ClusterSorting)
    return acc
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    flags.Detector.EnableCalo = False
    flags.FPGADataPrep.DoActs = True
    
    # DataPreparation Pipeline doesn't do spacepoint fomration, we need ACTS to do it
    flags.FPGADataPrep.PassThrough.ClusterOnly = True
    # For Spacepoint formation
    if flags.FPGADataPrep.PassThrough.ClusterOnly:
        flags.Acts.useCache = False
        flags.Tracking.ITkMainPass.doActsSeed = True
    
    flags.Tracking.ITkMainPass.doAthenaToActsCluster = True
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint = True
    flags.Tracking.ITkMainPass.doAthenaSpacePoint = True
    from ActsConfig.ActsCIFlags import actsLegacyWorkflowFlags
    actsLegacyWorkflowFlags(flags)
    flags.Acts.doRotCorrection = False
    
    flags.Concurrency.NumThreads = 1
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"]
    # flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    flags.Output.AODFileName = "FPGA.Benchmark.AOD.pool.root"
    flags.Debug.DumpEvtStore=False
    flags.fillFromArgs()
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)

    kwarg = {}
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    
            
    #Truth
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))
            
    # Standard reco
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
    cfg.merge(ITkTrackRecoCfg(flags))
        
    from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
    cfg.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

    acc = BenchmarkCfg(flags, **kwarg)
    cfg.merge(acc)
    
    OutputItemList = [
                    "xAOD::StripClusterContainer#FPGAStripClusters",
                    "xAOD::StripClusterAuxContainer#FPGAStripClustersAux.",
                    "xAOD::PixelClusterContainer#FPGAPixelClusters",
                    "xAOD::PixelClusterAuxContainer#FPGAPixelClustersAux.",
                    ]
    
    # Connection to ACTS
    if flags.FPGADataPrep.DoActs:
        
        # convert xAOD Clusters to SPs
        from EFTrackingFPGAUtility.DataPrepToActsConfig import UseActsSpacePointFormationCfg
        cfg.merge(UseActsSpacePointFormationCfg(flags))
        
        # Sort FPGAClusters
        cfg.merge(FPGAClusterSortingCfg(flags))
        
        # Run the ACTS Fast Tracking on FPGA clusters
        from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimDataPrepConnectToFastTracking
        cfg.merge(FPGATrackSimDataPrepConnectToFastTracking(flags, FinalTracks="FPGA",
                            **{'PixelSeedingAlg.InputSpacePoints' : ['FPGAPixelSpacePoints'],
                                'StripSeedingAlg.InputSpacePoints' : [''],
                                'TrackFindingAlg.UncalibratedMeasurementContainerKeys' : ["SortedFPGAPixelClusters","SortedFPGAStripClusters"],
                                'PixelClusterToTruthAssociationAlg.Measurements' : 'SortedFPGAPixelClusters',
                                'StripClusterToTruthAssociationAlg.Measurements' : 'SortedFPGAStripClusters'}))
        
        # Run the ACTS Fast Tracking (C-100) as an additional reference
        cfg.merge(FPGATrackSimDataPrepConnectToFastTracking(flags, FinalTracks="ActsFast"))
        
        OutputItemList += [
                    "xAOD::TrackParticleContainer#FPGATrackParticles",
                    "xAOD::TrackParticleAuxContainer#FPGATrackParticlesAux."
                    ]
        
        # This part is needed to extract technical efficiency
        from InDetConfig.InDetPrepRawDataToxAODConfig import ITkActsPrepDataToxAODCfg
        cfg.merge( ITkActsPrepDataToxAODCfg( flags,
                    PixelMeasurementContainer = "ITkPixelMeasurements_offl",
                    StripMeasurementContainer = "ITkStripMeasurements_offl" ) )
        OutputItemList += ['xAOD::TrackMeasurementValidationContainer#ITkPixelMeasurements_offl',
                            'xAOD::TrackMeasurementValidationAuxContainer#ITkPixelMeasurements_offlAux.',
                            'xAOD::TrackMeasurementValidationContainer#ITkStripMeasurements_offl',
                            'xAOD::TrackMeasurementValidationAuxContainer#ITkStripMeasurements_offlAux.']

    from EFTrackingFPGAOutputValidation.FPGAOutputValidationConfig import FPGAOutputValidationCfg
    cfg.merge(FPGAOutputValidationCfg(flags, **{
        "pixelKeys": ["FPGAPixelClusters", "ITkPixelClusters"],
        "stripKeys": ["FPGAStripClusters", "ITkStripClusters"],
        'doDiffHistograms':True,
        'matchByID' : True,
        'allowedRdoMisses': 1000}))
    
    
    # Prepare output
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory
    cfg.merge(SetupMetaDataForStreamCfg(flags,"AOD", 
                                        createMetadata=[
                                        MetadataCategory.ByteStreamMetaData,
                                        MetadataCategory.LumiBlockMetaData,
                                        MetadataCategory.TruthMetaData,
                                        MetadataCategory.IOVMetaData,],))

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    cfg.merge(addToAOD(flags, OutputItemList))

    cfg.run(flags.Exec.MaxEvents)
    
