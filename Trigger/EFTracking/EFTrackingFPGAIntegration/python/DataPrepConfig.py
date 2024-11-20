# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def xAODContainerMakerCfg(flags, name = 'xAODContainerMaker', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('OutputStripName', 'FPGAStripClusters')
    kwarg.setdefault('OutputPixelName', 'FPGAPixelClusters')
    kwarg.setdefault('OutputStripSpacePointName', 'FPGAStripSpacePoints')
    kwarg.setdefault('OutputPixelSpacePointName', 'FPGAPixelSpacePoints')
    
    acc.setPrivateTools(CompFactory.xAODContainerMaker(**kwarg))
    return acc

def PassThroughToolCfg(flags, name = 'PassThroughTool', **kwarg):
        
    acc = ComponentAccumulator()
        
    kwarg.setdefault('name', name)
    kwarg.setdefault('StripClusterContainerKey', 'ITkStripClusters')
    kwarg.setdefault('PixelClusterContainerKey', 'ITkPixelClusters')
    kwarg.setdefault('RunSW', True)
        
    acc.setPrivateTools(CompFactory.PassThroughTool(**kwarg))
    return acc

def DataPrepCfg(flags, name = "DataPreparationPipeline", **kwarg):

    acc = ComponentAccumulator()
    
    containerMakerTool = acc.popToolsAndMerge(xAODContainerMakerCfg(flags))
    passThroughTool = acc.popToolsAndMerge(PassThroughToolCfg(flags))
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('KernelName', '')
    kwarg.setdefault('RunPassThrough', False)
    kwarg.setdefault('xAODMaker', containerMakerTool)
    kwarg.setdefault('PassThroughTool', passThroughTool)

    acc.addEventAlgo(CompFactory.DataPreparationPipeline(**kwarg))
    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    # The input file should be specified by the user
    flags.Input.Files = [""]
    flags.Output.AODFileName = "DataPrepAOD.pool.root"

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    
    kwarg = {}
    kwarg["OutputLevel"] = DEBUG

    acc = DataPrepCfg(flags, **kwarg)
    cfg.merge(acc)
    
    # Add the AOD output stream
    # This is only for temporary development purposes
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    OutputItemList = ["xAOD::StripClusterContainer#FPGAStripClusters",
                     "xAOD::StripClusterAuxContainer#FPGAStripClustersAux.",
                     "xAOD::PixelClusterContainer#FPGAPixelClusters",
                     "xAOD::PixelClusterAuxContainer#FPGAPixelClustersAux."
                     ]
   
    cfg.merge(addToAOD(flags, OutputItemList))

    cfg.run(1)
