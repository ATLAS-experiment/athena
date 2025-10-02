# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def SpacepointsCfg(flags, **kwarg):
    acc=ComponentAccumulator()

    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('KernelName', 'dut')
    kwarg.setdefault('InputTV', '')
    kwarg.setdefault('RefTV', '')

    acc.addEventAlgo(CompFactory.Spacepoints(**kwarg))

    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    ConfigFlags = initConfigFlags()
    ConfigFlags.Concurrency.NumThreads = 1
    ConfigFlags.lock()

    cfg=MainServicesCfg(ConfigFlags)

    acc=SpacepointsCfg(ConfigFlags, OutputLevel=DEBUG)
    cfg.merge(acc)

    cfg.run(1)