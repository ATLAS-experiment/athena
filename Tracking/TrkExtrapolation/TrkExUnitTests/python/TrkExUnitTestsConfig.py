# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Define methods to configure TrkExUnitTest"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PositionMomentumWriterCfg(flags, name="PositionMomentumWriter", **kwargs):
  result = ComponentAccumulator()
  result.setPrivateTools(CompFactory.Trk.PositionMomentumWriter(name, **kwargs))
  return result

def ExtrapolationEngineTestCfg(flags, name = "ExtrapolationEngineTest", **kwargs):
  result=ComponentAccumulator()  

  histSvc = CompFactory.THistSvc(Output = [
    "val DATAFILE='ExtrapolationEngineTest.root' TYPE='ROOT' OPT='RECREATE'"])
  result.addService( histSvc )    

  from TrkConfig.AtlasExtrapolationEngineConfig import AtlasExtrapolationEngineCfg
  kwargs.setdefault("ExtrapolationEngine", result.getPrimaryAndMerge(
    AtlasExtrapolationEngineCfg(flags)))

  kwargs.setdefault('PositionMomentumWriter', result.popToolsAndMerge(
    PositionMomentumWriterCfg(flags)))

  result.addEventAlgo(CompFactory.Trk.ExtrapolationEngineTest(name, **kwargs))
  return result
