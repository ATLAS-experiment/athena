# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ForwardTransportModelCfg(flags, name="ForwardTransportModel", **kwargs):
    result = ComponentAccumulator()
    from ForwardTransportSvc.ForwardTransportSvcConfig import ForwardTransportSvcCfg
    serviceName ="ForwardTransportSvc"
    result.merge(ForwardTransportSvcCfg(flags, serviceName, **kwargs))
    kwargs.setdefault("ForwardTransportSvcName", serviceName) #Not a ServiceHandle
    kwargs.setdefault("RegionName" , "FWDBeamLine")
    result.setPrivateTools(CompFactory.ForwardTransportModelTool(name, **kwargs))
    return result
