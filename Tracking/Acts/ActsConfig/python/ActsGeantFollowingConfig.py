# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsGeantFollowerToolCfg(flags,
                             name: str = "ActsGeantFollowerTool",
                             **kwargs) -> ComponentAccumulator:
  THistSvc= CompFactory.THistSvc
  acc = ComponentAccumulator()
  histsvc = THistSvc(name="THistSvc")
  histsvc.Output = ["val DATAFILE='GeantFollowing.root' OPT='RECREATE'"]
  acc.addService(histsvc)
  acc.setPrivateTools(CompFactory.ActsGeantFollowerTool(name, **kwargs))  
  return acc
