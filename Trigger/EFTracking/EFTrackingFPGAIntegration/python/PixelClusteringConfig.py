# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def PixelClusteringCfg(flags, name = 'PixelClustering', **kwarg):
    acc=ComponentAccumulator()

    kwarg.setdefault('name', name)
    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('KernelName', 'clustering')
    kwarg.setdefault('InputTV', '')
    kwarg.setdefault('RefTV', '')

    acc.addEventAlgo(CompFactory.PixelClustering(**kwarg))

    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    
    kwarg = {}
    kwarg["OutputLevel"] = DEBUG

    acc=PixelClusteringCfg(flags, **kwarg)
    cfg.merge(acc)

    cfg.run(1)