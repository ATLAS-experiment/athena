# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def F100IntegrationCFG(flags, name = 'F100IntegrationAlog', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('PixelL2GKernelName','l2g_pixel_tool')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('EDMPrepKernelName', 'EDMPrep')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')
    kwarg.setdefault('FPGAThreads', flags.Concurrency.NumThreads)
    kwarg.setdefault('doEmulation', flags.FPGADataPrep.DoEmulation)
    kwarg.setdefault('doF110', flags.FPGADataPrep.DoF110)
    
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

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F100IntegrationAlg(**kwarg))

    return acc

def FPGAClusterSortingCfg(flags,**kwargs):
    acc = ComponentAccumulator()
    from FPGAClusterSorting.FPGAClusterSortingConfig import FPGAClusterSortingAlgCfg
    ClusterSorting = FPGAClusterSortingAlgCfg(flags,**kwargs)
    
    acc.merge(ClusterSorting)
    return acc

def F100FlagsCfg(flags):
    flags.Concurrency.NumThreads=1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False
    
    from EFTrackingFPGAPipeline.IntegrationConfigFlag import addFPGADataPrepFlags
    addFPGADataPrepFlags(flags)
    
    return flags


def FPGADataPreparation(flags): # thsi is used to run the F100 through Reco_tf
    acc = ComponentAccumulator()
    acc.merge(F100IntegrationCFG(flags))
    acc.merge(FPGAClusterSortingCfg(flags,**{'sortedxAODPixelClusterContainer': 'ITkPixelClusters',
                                             'sortedxAODStripClusterContainer': 'ITkStripClusters'}))
    
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementStatusAlgCfg
    acc.merge(ITkPixelDetectorElementStatusAlgCfg(flags))
    
    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripDetectorElementStatusAlgCfg
    acc.merge(ITkStripDetectorElementStatusAlgCfg(flags))

    if flags.Acts.EDM.PersistifyClusters or flags.Acts.EDM.PersistifySpacePoints:
        toAOD = []

        pixel_cluster_shortlist = ['-pixelClusterLink']
        strip_cluster_shortlist = ['-sctClusterLink']
        
        pixel_cluster_variables = '.'.join(pixel_cluster_shortlist)
        strip_cluster_variables = '.'.join(strip_cluster_shortlist)

        toAOD += ['xAOD::PixelClusterContainer#ITkPixelClusters',
                  'xAOD::PixelClusterAuxContainer#ITkPixelClustersAux.' + pixel_cluster_variables,
                  'xAOD::StripClusterContainer#ITkStripClusters',
                  'xAOD::StripClusterAuxContainer#ITkStripClustersAux.' + strip_cluster_variables]
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
        acc.merge(addToAOD(flags, toAOD))
    return acc
    
    
    
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    # Add FPGA Integration flags
    from EFTrackingFPGAPipeline.IntegrationConfigFlag import addFPGADataPrepFlags
    addFPGADataPrepFlags(flags)
    
    flags.Detector.EnableCalo = False
    flags.FPGADataPrep.DoActs = True
    flags.Acts.doRotCorrection = False
    
    flags.Concurrency.NumThreads = 1
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"]
    # flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    flags.Output.AODFileName = "FPGA.Benchmark.AOD.pool.root"
    flags.Debug.DumpEvtStore=False
    flags.fillFromArgs()

    if(not flags.FPGADataPrep.ForTiming):
        # DataPreparation Pipeline doesn't do spacepoint fomration, we need ACTS to do it
        flags.FPGADataPrep.PassThrough.ClusterOnly = True
        # For Spacepoint formation
        if flags.FPGADataPrep.PassThrough.ClusterOnly:
            flags.Acts.useCache = False
            flags.Tracking.ITkMainPass.doActsSeed = True
        
        flags.Tracking.ITkMainPass.doAthenaToActsCluster = True
        flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint = True
        flags.Tracking.ITkMainPass.doAthenaSpacePoint = True
    else:
        flags.Tracking.doTruth=False
        flags.ITk.doTruth=False
        flags.InDet.doTruth=False

    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)

    kwarg = {}
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
 
    if(not flags.FPGADataPrep.ForTiming):            
        #Truth
        if flags.Input.isMC:
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            cfg.merge(GEN_AOD2xAODCfg(flags))
                
        # Standard reco
        from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
        cfg.merge(ITkTrackRecoCfg(flags))

        from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
        cfg.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

    else:
        from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementStatusAlgCfg
        cfg.merge(ITkPixelDetectorElementStatusAlgCfg(flags))


    acc = F100IntegrationCFG(flags, **kwarg)
    cfg.merge(acc)
    
    OutputItemList = []
    # # Connection to ACTS
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
        if(not flags.FPGADataPrep.ForTiming):     

            # Run the ACTS Fast Tracking (C-100) as an additional reference
            cfg.merge(FPGATrackSimDataPrepConnectToFastTracking(flags, FinalTracks="ActsFast"))
            
            OutputItemList += [
                        "xAOD::TrackParticleContainer#FPGATrackParticles",
                        "xAOD::TrackParticleAuxContainer#FPGATrackParticlesAux."
                        ]


    if(not flags.FPGADataPrep.ForTiming):     
        OutputItemList += [
                        "xAOD::StripClusterContainer#FPGAStripClusters",
                        "xAOD::StripClusterAuxContainer#FPGAStripClustersAux.",
                        "xAOD::PixelClusterContainer#FPGAPixelClusters",
                        "xAOD::PixelClusterAuxContainer#FPGAPixelClustersAux.",
                        ]

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

    flags.dump()

    from AthenaCommon.Constants import DEBUG
    cfg.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    cfg.printConfig(withDetails=True, summariseProps=True)
    cfg.store(open("F100IntegrationAlg.pkl", "wb"))
    cfg.run(flags.Exec.MaxEvents)



