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

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.BenchmarkAlg(**kwarg))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    # dummy input to retrieve EventInfo
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"]
    flags.Output.AODFileName = "PassthroughAOD.pool.root"
    
    flags.lock()

    kwarg = {}
    
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    acc = BenchmarkCfg(flags, **kwarg)
    cfg.merge(acc)
    
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
    OutputItemList = [
                    "xAOD::StripClusterContainer#FPGAStripClusters",
                    "xAOD::StripClusterAuxContainer#FPGAStripClustersAux.",
                    "xAOD::PixelClusterContainer#FPGAPixelClusters",
                    "xAOD::PixelClusterAuxContainer#FPGAPixelClustersAux.",
                    ]
   
    cfg.merge(addToAOD(flags, OutputItemList))

    cfg.run(1)