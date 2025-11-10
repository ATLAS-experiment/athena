# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def F1X0IntegrationCfg(flags, name = 'F1X0IntegrationAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    if(flags.FPGADataPrep.doF110):
        kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    else:
        kwarg.setdefault('PixelClusterKernelName', 'pixclustering_top_v1_0')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('PixelL2GKernelName','l2g_pixel_tool')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')
    kwarg.setdefault('doF110', flags.FPGADataPrep.doF110)

    if ("isRoI_Seeded" in kwarg) and kwarg["isRoI_Seeded"]:
        if 'RegSelTool' not in kwarg:
            from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
            kwarg.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    montool = GenericMonitoringTool(flags, HistPath = f"F100Integration_{name}")
    montool.defineHistogram('TIME_Total',path='EXPERT',type='TH1F',title="Total time (ms)", xbins = 400, xmin=0.0, xmax=2000.0)

    kwarg.setdefault('MonTool', montool)

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F1X0IntegrationAlg(name, **kwarg))

    return acc

def F1X0XRTIntegrationCfg(flags, name = 'F1X0IntegrationAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    if(flags.FPGADataPrep.doF110):
        kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    else:
        kwarg.setdefault('PixelClusterKernelName', 'pixclustering_top_v1_0')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('PixelL2GKernelName','l2g_pixel_tool')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')
    kwarg.setdefault('doF110', flags.FPGADataPrep.doF110)

    if ("isRoI_Seeded" in kwarg) and kwarg["isRoI_Seeded"]:
        if 'RegSelTool' not in kwarg:
            from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
            kwarg.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F1X0XRTIntegrationAlg(name, **kwarg))

    return acc

def F100StreamIntegrationCfg(flags, name = 'F100StreamIntegrationAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    kwarg.setdefault('PixelStartClusterKernelName','loaderPixel')
    kwarg.setdefault('PixelEndClusterKernelName','unloaderPixelCluster')
    kwarg.setdefault('PixelEndClusterEdmKernelName','unloaderPixelEdm')

    kwarg.setdefault('StripStartClusterKernelName','loaderStrip')
    kwarg.setdefault('StripEndClusterKernelName','unloaderStrip')
    kwarg.setdefault('PixelL2GKernelName','l2g_pixel_tool')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')

    if ("isRoI_Seeded" in kwarg) and kwarg["isRoI_Seeded"]:
        if 'RegSelTool' not in kwarg:
            from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
            kwarg.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F100StreamIntegrationAlg(name, **kwarg))

    return acc

def F110IntegrationCfg(flags, name = 'F110IntegrationAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')

    if ("isRoI_Seeded" in kwarg) and kwarg["isRoI_Seeded"]:
        if 'RegSelTool' not in kwarg:
            from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
            kwarg.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F110IntegrationAlg(name, **kwarg))

    return acc

def F110StreamIntegrationCfg(flags, name = 'F110StreamIntegrationAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', flags.FPGADataPrep.xclbin)
    kwarg.setdefault('PixelStartClusterKernelName','pixelLoader')
    kwarg.setdefault('PixelEndClusterKernelName','pixelUnloader')

    kwarg.setdefault('StripStartClusterKernelName','stripLoader')
    kwarg.setdefault('StripEndClusterKernelName','stripUnloader')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')

    if ("isRoI_Seeded" in kwarg) and kwarg["isRoI_Seeded"]:
        if 'RegSelTool' not in kwarg:
            from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
            kwarg.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F110StreamIntegrationAlg(name, **kwarg))

    return acc


def F100DataEncodingCfg(flags, name = 'F100DataEncodingAlg', **kwarg):
    acc = ComponentAccumulator()

    # Set up Cluster maker tool
    if("FPGADataFormatTool" not in kwarg):
        from EFTrackingFPGAPipeline.DataPrepConfig import FPGADataFormatToolCfg
        dataFormatTool = acc.popToolsAndMerge(FPGADataFormatToolCfg(flags))
        kwarg.setdefault('FPGADataFormatTool', dataFormatTool)

    kwarg.setdefault('isRoI_Seeded', False)
    
    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F100DataEncodingAlg(name, **kwarg))

    return acc

def F100EDMConversionCfg(flags, name = 'F100EDMConversionAlg', **kwarg):
    acc = ComponentAccumulator()
    
    # Set up Cluster maker tool
    if("xAODClusterMaker" not in kwarg):
        from EFTrackingFPGAPipeline.DataPrepConfig import xAODClusterMakerCfg
        clusterMakerTool = acc.popToolsAndMerge(xAODClusterMakerCfg(flags))
        kwarg.setdefault('xAODClusterMaker', clusterMakerTool)

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.F100EDMConversionAlg(name, **kwarg))

    return acc


def FPGAClusterSortingCfg(flags,**kwargs):
    acc = ComponentAccumulator()
    from FPGAClusterSorting.FPGAClusterSortingConfig import FPGAClusterSortingAlgCfg
    ClusterSorting = FPGAClusterSortingAlgCfg(flags,**kwargs)
    
    acc.merge(ClusterSorting)
    return acc

def F100FlagsCfg(flags):
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False

    return flags


def FPGADataPreparation(flags,runStandalone=False): # thsi is used to run the F100 through Reco_tf
    kwargs = {}
    kwargs.setdefault('FPGAThreads', flags.Concurrency.NumThreads)
    acc = ComponentAccumulator()
    acc.merge(F100DataEncodingCfg(flags))
    
    if(flags.FPGADataPrep.doCodeType == "F1X0"):
        acc.merge(F1X0IntegrationCfg(flags, "F1X0IntegrationAlg", **kwargs))
    elif(flags.FPGADataPrep.doCodeType == "F1X0XRT"):
        acc.merge(F1X0XRTIntegrationCfg(flags, "F1X0XRTIntegrationAlg", **kwargs))
    elif(flags.FPGADataPrep.doCodeType == "F100Stream"):
        acc.merge(F100StreamIntegrationCfg(flags, "F100StreamIntegrationAlg", **kwargs))
    elif(flags.FPGADataPrep.doCodeType == "F110"):
        acc.merge(F110IntegrationCfg(flags, "F110IntegrationAlg", **kwargs))
    elif(flags.FPGADataPrep.doCodeType == "F110Stream"):
        acc.merge(F110StreamIntegrationCfg(flags, "F110StreamIntegrationAlg", **kwargs))
    else:
        print("Code Type is not recognized")
        exit(1)

    acc.merge(F100EDMConversionCfg(flags))
    acc.merge(FPGAClusterSortingCfg(flags,**{'sortedxAODPixelClusterContainer': 'SortedFPGAPixelClusters' if runStandalone else 'ITkPixelClusters',
                                             'sortedxAODStripClusterContainer': 'SortedFPGAStripClusters' if runStandalone else 'ITkStripClusters'}))

    if(not runStandalone):
        if(not flags.FPGADataPrep.ForTiming): 
            from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
            acc.merge(FPGATrackSimReportingCfg(flags,
                                               perEventReports = False, # set to True if per-event information is needed for debugging (e.g. cluster, tracks). Otherwise it produces a lot of output
                                            **{'xAODPixelClusterContainers' : ['ITkPixelClusters'],
                                                'xAODStripClusterContainers' : ['ITkStripClusters'],
                                                'FPGAActsTracks' : [],
                                                'isDataPrep': True} ))
        
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

    cfg.merge(FPGADataPreparation(flags,runStandalone=True))

    OutputItemList = []
    # # Connection to ACTS
    if flags.FPGADataPrep.DoActs:
        
        # convert xAOD Clusters to SPs
        from EFTrackingFPGAUtility.DataPrepToActsConfig import UseActsSpacePointFormationCfg
        cfg.merge(UseActsSpacePointFormationCfg(flags))
    

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
            'matchByID' : False,
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



