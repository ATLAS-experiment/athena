# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MetaDataSvcCfg(flags, toolNames=[], tools=[]):
    result = ComponentAccumulator()

    result.addService(CompFactory.StoreGateSvc("MetaDataStore"))
    result.addService(CompFactory.StoreGateSvc("InputMetaDataStore"))

    service = CompFactory.MetaDataSvc("MetaDataSvc",
                                      MetaDataContainer="MetaDataHdr",
                                      ConversionService="AthenaPoolSharedIOCnvSvc" if flags.MP.UseSharedReader or flags.MP.UseSharedWriter else "AthenaPoolCnvSvc")
    result.addService(service)
    result.addService(CompFactory.ProxyProviderSvc(ProviderNames=["MetaDataSvc"]))

    for tool in tools:
        result.addPublicTool(tool)
        service.MetaDataTools += [tool]

    for name in toolNames:
        if not isinstance(name, str):
            from AthenaCommon.Logging import logging
            log = logging.getLogger("MetaDataSvcConfig")
            log.error('Attempted to pass a non-string argument as a metadata tool name')
            continue
        tool = CompFactory.getComp(name)()
        result.addPublicTool(tool)
        service.MetaDataTools += [tool]

    return result
