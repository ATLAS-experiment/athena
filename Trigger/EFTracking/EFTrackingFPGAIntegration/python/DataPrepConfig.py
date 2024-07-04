# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def ClusterContainerMakerCfg(flags, name = 'ClusterContainerMaker', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('OutputStripName', 'ITkStripClusters')
    kwarg.setdefault('OutputPixelName', 'ITkPixelClusters')
    
    acc.setPrivateTools(CompFactory.ClusterContainerMaker(**kwarg))
    return acc

def DataPrepCfg(flags, name = "DataPreparationPipeline", **kwarg):

    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(ClusterContainerMakerCfg(flags))
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('xclbin', './xAODTransfer.xclbin')
    kwarg.setdefault('KernelName', 'xAODTransfer')
    kwarg.setdefault('ClusterMaker', tool)
    kwarg.setdefault('StripClusterContainerKey', '')
    kwarg.setdefault('PixelClusterContainerKey', '')

    acc.addEventAlgo(CompFactory.DataPreparationPipeline(**kwarg))
    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    # Use a dummy input file for the EventInfo
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
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
    OutputItemList = ["xAOD::StripClusterContainer#ITkStripClusters",
                     "xAOD::StripClusterAuxContainer#ITkStripClustersAux.",
                     "xAOD::PixelClusterContainer#ITkPixelClusters",
                     "xAOD::PixelClusterAuxContainer#ITkPixelClustersAux."
                     ]
   
    cfg.merge(addToAOD(flags, OutputItemList))

    cfg.run(1)
