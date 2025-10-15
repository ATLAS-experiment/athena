# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def KernelTesterCfg(flags, name = 'F150BenchmarkAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', '/eos/project-a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F150_i/kernels.hw.xclbin')
    
    kwarg.setdefault('RunSlicing', True) 
    kwarg.setdefault('RunInsideOut', False) 
    kwarg.setdefault('RunInsideOutOnSlicingEngine', False) 
    kwarg.setdefault('RunFullF150', False) 

    kwarg.setdefault('SlicingEngineInputName', 'configurableLengthWideLoader') 
    kwarg.setdefault('SlicingEngineOutputName', 'dynamicLengthWideUnloader') 

    kwarg.setdefault('InsideOutInputName', 'mem_read') 
    kwarg.setdefault('InsideOutOutputName', 'mem_write') 

    kwarg.setdefault('PixelClusterKernelName','pixel_clustering_tool')
    kwarg.setdefault('StripClusterKernelName','processHits')
    kwarg.setdefault('StripL2GKernelName','l2g_strip_tool')
    kwarg.setdefault('PixelEDMPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEDMPrepKernelName', 'StripEDMPrep')


    # Set up Cluster maker tool
    from EFTrackingFPGAPipeline.DataPrepConfig import xAODClusterMakerCfg
    clusterMakerTool = acc.popToolsAndMerge(xAODClusterMakerCfg(flags))
    kwarg.setdefault('xAODClusterMaker', clusterMakerTool)
    
    # Set up TestVectorTool
    from EFTrackingFPGAUtility.FPGADataFormatter import FPGATestVectorToolCfg
    testVectorTool = acc.popToolsAndMerge(FPGATestVectorToolCfg(flags))
    kwarg.setdefault('TestVectorTool', testVectorTool)
 
    outputTool = acc.popToolsAndMerge(FPGAOutputConversionToolCfg(flags))
    kwarg.setdefault('OutputConversionTool', outputTool)
    
    # Set up Chrono service
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    alg = CompFactory.EFTrackingFPGAIntegration.F150KernelTesterAlg(**kwarg)
    acc.addEventAlgo(alg)

    return acc

def F150EDMConversionAlgCfg(flags, **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('FPGAOutputTrackKey', "FPGATrackOutput")
    kwarg.setdefault('FPGASpacePointsKey', "ITkPixelSpacePoints")
    kwarg.setdefault('OutputSeeds', "ActsValidateF150PixelSeeds")

    alg = CompFactory.EFTrackingFPGAIntegration.F150EDMConversionAlg(**kwarg)
    acc.addEventAlgo(alg)

    return acc


def FPGAOutputConversionToolCfg(flags, name = 'FPGAOutputConversionTool', **kwarg):

    acc = ComponentAccumulator()

    kwarg.setdefault('name', name)
    acc.setPrivateTools(CompFactory.OutputConversionTool(**kwarg))

    return acc



if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    
    ############################################
    # Flags used in the prototrack chain
    FinalProtoTrackChainxAODTracksKey="FPGA"
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
    from ActsConfig.ActsCIFlags import actsLegacyWorkflowFlags
    actsLegacyWorkflowFlags(flags)
    flags.Acts.doRotCorrection = False

    ############################################
    flags.Concurrency.NumThreads=1
    #flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False
    # single muon
    #flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"]
    # ttbar
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    
    flags.fillFromArgs()
    
    # Additional (necessary) flag re-configuration for mutliregion tracking
    from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import ConfigureMultiRegionFlags
    ConfigureMultiRegionFlags(flags)
    
    flags.Trigger.FPGATrackSim.tracking = False
    flags.Trigger.FPGATrackSim.Hough.genScan = True
    flags.Trigger.FPGATrackSim.Hough.secondStage = False

    flags.lock()
    flags.dump()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
    cfg=MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

        from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
        cfg.merge(addTruthPileupJetsToOutputCfg(flags))

    if flags.Detector.EnableCalo:
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        cfg.merge(CaloRecoCfg(flags))

    if flags.Tracking.recoChain:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        cfg.merge(InDetTrackRecoCfg(flags))

    
    # Configure both the dataprep and logical hits algorithms.
    from InDetConfig.InDetPrepRawDataFormationConfig import ITkXAODToInDetClusterConversionCfg
    cfg.merge(ITkXAODToInDetClusterConversionCfg(flags)) # needed for the FPGATrackSim DataPrep to work
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import  FPGATrackSimDataPrepAlgCfg
    cfg.merge(FPGATrackSimDataPrepAlgCfg(flags))
    from FPGATrackSimConfTools.FPGATrackSimMultiRegionConfig import FPGATrackSimMultiRegionTrackingCfg
    cfg.merge(FPGATrackSimMultiRegionTrackingCfg(flags))

    kwarg = {}
    acc = KernelTesterCfg(flags, **kwarg)
    cfg.merge(acc)    

    from AthenaCommon.Constants import DEBUG
    cfg.foreach_component("AthEventSeq/*").OutputLevel = DEBUG

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.run(flags.Exec.MaxEvents)
    
