# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg

def BackgroundAlgsCfg(flags):

  result=ComponentAccumulator()

  result.merge(LArCollisionTimeCfg(flags))
  from AthenaConfiguration.Enums import BeamType
  if flags.Beam.Type is BeamType.Collisions:
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSegmentSelectionToolCfg
    result.addEventAlgo(CompFactory.BeamBackgroundFiller(SegmentSelector = result.popToolsAndMerge(MuonSegmentSelectionToolCfg(flags))))
  result.addEventAlgo(CompFactory.BcmCollisionTimeAlg())

  result.addEventAlgo(CompFactory.BackgroundWordFiller(IsMC=flags.Input.isMC))

  return result

#No self-test __main__ here because these algos depend on many event data objects not stored in ESD, therefore they can't run stand-alone. 
