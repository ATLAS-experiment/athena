# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def BenchmarkCfg(flags, name = 'BenckmarkAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('kernelName', '')
    kwarg.setdefault('InputPixelClusterKey', 'ITkPixelClusters')
    kwarg.setdefault('InputStripClusterKey', 'ITkStripClusters')
    
    from EFTrackingFPGAIntegration.DataPrepConfig import xAODContainerMakerCfg
    containerMakerTool = acc.popToolsAndMerge(xAODContainerMakerCfg(flags))
    kwarg.setdefault('xAODContainerMaker', containerMakerTool)
    
    from EFTrackingFPGAIntegration.FPGADataFormatter import FPGATestVectorToolCfg
    testVectorTool = acc.popToolsAndMerge(FPGATestVectorToolCfg(flags))
    kwarg.setdefault('TestVectorTool', testVectorTool)

    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True
    ))

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.BenchmarkAlg(**kwarg))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    # Add FPGA Integration flags
    from EFTrackingFPGAIntegration.IntegrationConfigFlag import addFPGADataPrepFlags
    addFPGADataPrepFlags(flags)
    
    flags.Detector.EnableCalo = False
    flags.FPGADataPrep.DoActs = True
    
    # DataPreparation Pipeline doesn't do spacepoint fomration, we need ACTS to do it
    flags.FPGADataPrep.PassThrough.ClusterOnly = True
    # For Spacepoint formation
    if flags.FPGADataPrep.PassThrough.ClusterOnly:
        flags.Detector.EnableITkPixel = True
        flags.Detector.EnableITkStrip = True
        flags.Acts.useCache = False
        flags.Tracking.ITkMainPass.doActsSeed = True
    
    flags.Tracking.ITkMainPass.doAthenaToActsCluster = True
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint = True
    flags.Tracking.ITkMainPass.doAthenaSpacePoint = True
    
    flags.Concurrency.NumThreads = 1
    # dummy input to retrieve EventInfo
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"]
    flags.Output.AODFileName = "PassthroughAOD.pool.root"
    
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)

    kwarg = {}
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    
    # Check if the input file is RDO or AOD by instepecting if the file contains "RDO" or "AOD"
    isRDO = False
    if "RDO" in flags.Input.Files[0] or isRDO:
        print("The input is an RDO, running the ITk Reco chain") 
            
        #Truth
        if flags.Input.isMC:
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            cfg.merge(GEN_AOD2xAODCfg(flags))
            
        # Standard reco
        from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
        cfg.merge(ITkTrackRecoCfg(flags))
        
        from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
        cfg.merge( TruthParticleIndexDecoratorAlgCfg(flags) )
    
    elif "AOD" in flags.Input.Files[0]:
        print("The input is an AOD, skipping the ITk Reco chain")
    
    else:
        print("Cannot determine the input file type. Assuming it's an AOD. Set the isRDO option manully if it's not.")

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
        from EFTrackingFPGAIntegration.DataPrepToActsConfig import DataPrepToActsCfg
        cfg.merge(DataPrepToActsCfg(flags))
        OutputItemList += [
                    "xAOD::TrackParticleContainer#FPGATrackParticles",
                    "xAOD::TrackParticleAuxContainer#FPGATrackParticlesAux."
                    ]
    
    from EFTrackingFPGAIntegration.FPGAOutputValidationConfig import FPGAOutputValidationCfg
    cfg.merge(FPGAOutputValidationCfg(flags, **{
        "pixelKeys": ["FPGAPixelClusters", "ITkPixelClusters"],
        "stripKeys": ["FPGAStripClusters", "ITkStripClusters"],
    }))
    
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

    cfg.run(1)