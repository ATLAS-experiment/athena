# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhaseIIPixelRawDataContainerCfg(flags, **kwargs) :
    """
    supported kwargs:
    RDOKey:str = 'ITkPixelRDOs' name of the Pixel RDO container that should use the PhaseII container type.
    """
    acc = ComponentAccumulator()
    rdoKey = kwargs.pop("RDOKey","ITkPixelRDOs")
    AddressRemappingSvc = CompFactory.AddressRemappingSvc(
        TypeKeyOverwriteMaps = [f"PixelRDO_Container#{rdoKey}->PhaseIIPixelRawDataContainer#{rdoKey}" ]
        )
    acc.addService(AddressRemappingSvc)

    ProxyProviderSvc = CompFactory.ProxyProviderSvc(
        ProviderNames = [ AddressRemappingSvc.name ]
        )
    acc.addService(ProxyProviderSvc)
    return acc

def PhaseIIStripRawDataContainerCfg(flags, **kwargs) :
    """
    supported kwargs:
    RDOKey:str = 'ITkStripRDOs' name of the Strip RDO container that should use the PhaseII container type.
    """
    acc = ComponentAccumulator()
    rdoKey = kwargs.pop("RDOKey","ITkStripRDOs")
    AddressRemappingSvc = CompFactory.AddressRemappingSvc(
        TypeKeyOverwriteMaps = [f"SCT_RDO_Container#{rdoKey}->PhaseIIStripRawDataContainer#{rdoKey}" ]
        )
    acc.addService(AddressRemappingSvc)

    ProxyProviderSvc = CompFactory.ProxyProviderSvc(
        ProviderNames = [ AddressRemappingSvc.name ]
        )
    acc.addService(ProxyProviderSvc)
    return acc
