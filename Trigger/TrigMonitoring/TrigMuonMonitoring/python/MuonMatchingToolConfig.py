#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def MuonMatchingToolConfig(flags):
    from AthenaConfiguration.ComponentFactory import CompFactory

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from TrigT1MuctpiPhase1.TrigT1MuctpiPhase1Config import TrigThresholdDecisionToolCfg

    matchTool = CompFactory.MuonMatchingTool("MuonMatchingTool")
    matchTool.TrigThresholdDecisionTool = acc.popToolsAndMerge(TrigThresholdDecisionToolCfg(flags, 
                                                                                            name="TrigThresholdDecisionTool", 
                                                                                            AODinput = flags.Trigger.triggerConfig == 'INFILE'))
    acc.setPrivateTools(matchTool)

    return acc