# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AthenaPoolAddressProviderSvcCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.StoreGateSvc("MetaDataStore"))

    if flags.Input.SecondaryFiles:
        # These names and keys must match the two event selectors.
        providers = [
            dict(kwargs, name="AthenaPoolAddressProviderSvcPrimary",
                 DataHeaderKey="EventSelector", AttributeListKey="Input"),
            dict(kwargs, name="AthenaPoolAddressProviderSvcSecondary",
                 DataHeaderKey="SecondaryEventSelector"),
        ]
    else:
        providers = [kwargs]

    providerNames = []
    for properties in providers:
        service = CompFactory.AthenaPoolAddressProviderSvc(**properties)
        acc.addService(service)
        providerNames.append(service.getFullJobOptName())
    acc.addService(CompFactory.ProxyProviderSvc(ProviderNames=providerNames))
    return acc
