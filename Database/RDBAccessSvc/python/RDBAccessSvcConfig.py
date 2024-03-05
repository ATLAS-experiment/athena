
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def RDBAccessSvcCfg(flags, name="RDBAccessSvc", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.RDBAccessSvc(name,**kwargs), primary=True)
    return result