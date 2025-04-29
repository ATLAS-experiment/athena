# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PoolSvcCfg(flags, withCatalogs=False, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxFilesOpen", flags.PoolSvc.MaxFilesOpen)
    kwargs.setdefault("DefaultContainerType", flags.PoolSvc.DefaultContainerType)

    if withCatalogs:
        catalogs = [
            "apcfile:poolcond/PoolFileCatalog.xml",
            "apcfile:poolcond/PoolCat_oflcond.xml",
        ]

        if not flags.Input.isMC:
            catalogs += [
                "apcfile:poolcond/PoolCat_comcond.xml",
            ]

        kwargs.setdefault("ReadCatalog", catalogs)

    acc.addService(CompFactory.PoolSvc(**kwargs))
    return acc


def AthenaPoolCnvSvcCfg(flags, **kwargs):
    acc = PoolSvcCfg(flags)

    if flags.PoolSvc.PersSvcPerInputType:
        kwargs.setdefault("PersSvcPerInputType", "CollectionTree")

    service = CompFactory.AthenaPoolCnvSvc(**kwargs)
    acc.addService(service)
    acc.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",
                                                 CnvServices=[service.getFullJobOptName()]))
    return acc


def AthenaPoolAddressProviderSvcCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.StoreGateSvc("MetaDataStore"))

    service = CompFactory.AthenaPoolAddressProviderSvc(**kwargs)
    acc.addService(service)
    acc.addService(CompFactory.ProxyProviderSvc(ProviderNames=[service.getFullJobOptName()]))
    return acc
