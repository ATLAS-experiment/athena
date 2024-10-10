# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PartPropSvcCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("InputFile", "PDGTABLE.MeV")
    result.addService(CompFactory.PartPropSvc(name="PartPropSvc", **kwargs), primary=True)
    return result
