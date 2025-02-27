# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import INFO

def xAODContainerMakerCfg(flags, name = 'xAODContainerMaker', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('OutputStripName', 'FPGAStripClusters')
    kwarg.setdefault('OutputPixelName', 'FPGAPixelClusters')    
    # Spacepoints below will be further refined when the full pass-through kernel is ready
    kwarg.setdefault('OutputStripSpacePointName', 'PlaceHolderStripSpacePoints') 
    kwarg.setdefault('OutputPixelSpacePointName', 'PlaceHolderPixelSpacePoints')
    
    acc.setPrivateTools(CompFactory.xAODContainerMaker(**kwarg))
    return acc

def BenchmarkCfg(flags, name = 'BenckmarkAlg', **kwarg):
    acc = ComponentAccumulator()

    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('kernelName', '')
    kwarg.setdefault('PixelTestVectorKey', 'Pixel_EDM_TV')
    kwarg.setdefault('StripTestVectorKey', 'Strip_EDM_TV')
    
    containerMakerTool = acc.popToolsAndMerge(xAODContainerMakerCfg(flags))
    kwarg.setdefault('xAODContainerMaker', containerMakerTool)

    acc.addEventAlgo(CompFactory.EFTrackingFPGAIntegration.BenchmarkAlg(**kwarg))

    return acc

def PixelStreamLoaderCfg(flags, name = 'PixelStreamLoader', **kwarg):
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('bufferSize', 8192)
    kwarg.setdefault('inputDataStream', 'Pixel_EDM_TV')
    kwarg.setdefault('inputCsvPath', '')

    acc.addEventAlgo(CompFactory.EFTrackingDataStreamLoaderAlgorithm(**kwarg))

    return acc

def StripStreamLoaderCfg(flags, name = 'StripStreamLoader', **kwarg):
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('bufferSize', 8192)
    kwarg.setdefault('inputDataStream', 'Strip_EDM_TV')
    kwarg.setdefault('inputCsvPath', '')

    acc.addEventAlgo(CompFactory.EFTrackingDataStreamLoaderAlgorithm(**kwarg))

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
    kwarg["OutputLevel"] = INFO
    
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    acc = PixelStreamLoaderCfg(flags, **kwarg)
    cfg.merge(acc)
    
    acc = StripStreamLoaderCfg(flags, **kwarg)
    cfg.merge(acc)

    acc = BenchmarkCfg(flags, **kwarg)
    cfg.merge(acc)
    
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