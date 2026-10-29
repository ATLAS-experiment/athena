# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
import ROOT


def PoolReadCfg(flags):
    """
    Creates a ComponentAccumulator instance containing the
    athena services required for POOL file reading
    """

    result = ComponentAccumulator()

    if flags.MP.UseSharedReader or flags.MP.UseSharedWriter:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolSharedIOCnvSvcCfg
        result.merge(AthenaPoolSharedIOCnvSvcCfg(flags, InputPoolAttributes=["DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '-1'"]))
    elif flags.PoolSvc.PersSvcPerInputType: # Switch off caching when using many TFiles
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
        result.merge(AthenaPoolCnvSvcCfg(flags, InputPoolAttributes=["DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '0'"]))
    else:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
        result.merge(AthenaPoolCnvSvcCfg(flags, InputPoolAttributes=["DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '-1'"]))

    # Suppress ROOT warnings about old I/O classes.
    from AthenaServices.ROOTMessageFilterSvcConfig import ROOTMessageFilterSvcCfg
    result.merge(ROOTMessageFilterSvcCfg(flags,
                                         SuppressionRules=[('TClass::Init',
                                                            '.*DataHeader.*_p[12].*',
                                                            ROOT.kWarning),
                                                            ('TClass::Init',
                                                            '.*PoolToken_p1.*',
                                                            ROOT.kWarning)]))

    from EventSelectorAthenaPool.AthenaPoolAddressProviderSvcConfig import AthenaPoolAddressProviderSvcCfg
    result.merge(AthenaPoolAddressProviderSvcCfg(flags))
    from EventSelectorAthenaPool.EventSelectorAthenaPoolConfig import EventSelectorAthenaPoolCfg
    result.merge(EventSelectorAthenaPoolCfg(flags))
    evSel = result.getService("EventSelector")

    result.setAppProperty("EvtSel", evSel.getFullJobOptName())

    return result
