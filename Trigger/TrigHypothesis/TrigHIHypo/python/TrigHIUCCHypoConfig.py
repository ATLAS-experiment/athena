# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV

def TrigHIUCCHypoToolFromDict(flags, chainDict):
  """Configure the ultra-central collisions hypo tool"""

  tool = CompFactory.TrigHIUCCHypoTool(chainDict['chainName'])

  UCCInfo = chainDict['chainParts'][0]['hypoUCCInfo'][0]

  UCC_th = {"Th1":  3730*GeV,
            "Th2":  4280*GeV,
            "Th3":  4503*GeV }

  tool.FCalEtThreshold = UCC_th[UCCInfo.removeprefix('ucc')]

  return tool
