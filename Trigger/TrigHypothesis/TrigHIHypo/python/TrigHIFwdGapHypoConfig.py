# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def TrigHIFwdGapHypoToolFromDict(flags, chainDict):
  """Configure the FCal-based forward gap hypo tool"""

  tool = CompFactory.TrigHIFwdGapHypoTool(chainDict['chainName'])

  FgapInfo = chainDict['chainParts'][0]['hypoFgapInfo'][0]
  if 'AC' in FgapInfo:
    tool.maxFCalEt = int(FgapInfo.strip('FgapAC'))
    tool.useDoubleSidedGap = True
  elif 'A' in FgapInfo:
    tool.maxFCalEt = int(FgapInfo.strip('FgapA'))
    tool.useSideA = True
  elif 'C' in FgapInfo:
    tool.maxFCalEt = int(FgapInfo.strip('FgapC'))
    tool.useSideA = False

  return tool

