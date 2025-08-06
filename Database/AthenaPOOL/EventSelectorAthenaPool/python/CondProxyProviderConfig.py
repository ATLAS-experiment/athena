# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: EventSelectorAthenaPool/python/CondProxyProviderConfig.py
# Created: Jun 2020, sss
# Purpose: Configure CondProxyProvider.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CondProxyProviderCfg (flags, poolFiles):
    if not isinstance (poolFiles, list):
        poolFiles = [poolFiles]

    result = ComponentAccumulator()

    ProxyProviderSvc = CompFactory.ProxyProviderSvc # SGComps
    pps = ProxyProviderSvc (ProviderNames = ['CondProxyProvider'])
    result.addService (pps)
    
    CondProxyProvider = CompFactory.CondProxyProvider # EventSelectorAthenaPool
    cpp = CondProxyProvider (InputCollections = poolFiles,
                             ConversionService="AthenaPoolSharedIOCnvSvc" if flags.MP.UseSharedReader or flags.MP.UseSharedWriter else "AthenaPoolCnvSvc")
    result.addService (cpp)

    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import LHCPeriod
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.GeoModel.Run = LHCPeriod.Run2  # flags validation workaround
    flags.lock()

    acc1 = ComponentAccumulator()
    acc1.merge (CondProxyProviderCfg (flags, 'file1'))
    acc1.merge (CondProxyProviderCfg (flags, ['file2', 'file3']))
    acc1.printConfig (summariseProps=True)
    print ('CondProxyProvider:', acc1.getService('CondProxyProvider'))
    acc1.wasMerged()
