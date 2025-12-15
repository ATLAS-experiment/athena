# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def F600IntegrationCfg(flags, name = 'BenckmarkAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('bdfID', flags.FPGADataPrep.bdfID) # On the testbed
    kwarg.setdefault('xclbin', '/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F611/kernels.hw.xclbin')

    kwarg.setdefault('PixelClusterInputPath', '/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3a/DataPrep_FullDet_SingleMuon/pixel_cluster_input.txt')
    kwarg.setdefault('PixelStageOneSlicingInputPath', '/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3a/DataPrep_FullDet_SingleMuon/pixelL2G_output.txt')
    kwarg.setdefault('InsideOutInputPath', '/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3a/F600_Region34_SingleMuon/slicing_PixelFirst_output.txt')

# DataPrep
    kwarg.setdefault('PixelClusteringKernelName', 'pixel_clustering_tool')
    kwarg.setdefault('ProcessHitsKernelName', 'processHits')
    kwarg.setdefault('PixelL2gKernelName', 'l2g_pixel_tool')
    kwarg.setdefault('StripL2gKernelName', 'l2g_strip_tool')
    kwarg.setdefault('PixelEdmPrepKernelName', 'PixelEDMPrep')
    kwarg.setdefault('StripEdmPrepKernelName', 'StripEDMPrep')
    kwarg.setdefault('PixelFirstStageInputKernelName', 'krnl_input_stage_rtl')
    kwarg.setdefault('PixelFirstStageOutputKernelName', 'krnl_output_stage_rtl')

# Slicing Engine
    kwarg.setdefault('PixelFirstStageSlicingIPName', 'slicing_engine')

# Inside Out
    kwarg.setdefault('MemReadKernelName', 'mem_read')
    kwarg.setdefault('MemWriteKernelName', 'mem_write')

# Space Points
    kwarg.setdefault('SpacepointKernelName', 'spacepoint_tool')

# NN Pathfinder
    kwarg.setdefault('LoaderKernelName', 'loader')
    kwarg.setdefault('UnloaderKernelName', 'unloader')

# NN Classifier
    kwarg.setdefault('NnOverlapDecoratorKernelName', 'NNOverlapDecorator_kernel')

# Duplicate Remover
    kwarg.setdefault('RunnerKernelName', 'runner')

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

    alg = CompFactory.EFTrackingFPGAIntegration.F600IntegrationAlg(**kwarg)
    import ROOT
    alg.OutputLevel = ROOT.MSG.DEBUG
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
    
    if (flags.Trigger.FPGATrackSim.pipeline.startswith('F-6')):
        print("You are trying to run an F-6* pipeline! I am auto-configuring the Inside-Out for you. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.genScan=True
        flags.Trigger.FPGATrackSim.spacePoints = flags.Trigger.FPGATrackSim.Hough.secondStage
        print("You are trying to run the NN fake rejection as part of a pipeline! I am going to enable this for you whether you want to or not")
        flags.Trigger.FPGATrackSim.tracking = True
        flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = True

    elif (flags.Trigger.FPGATrackSim.pipeline != ""):
        raise AssertionError("ERROR You are trying to run the pipeline " + flags.Trigger.FPGATrackSim.pipeline + " which is not yet supported!")

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
    acc = F600IntegrationCfg(flags, **kwarg)
    cfg.merge(acc)    

    from AthenaCommon.Constants import DEBUG
    cfg.foreach_component("AthEventSeq/*").OutputLevel = DEBUG

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.run(flags.Exec.MaxEvents)
    
