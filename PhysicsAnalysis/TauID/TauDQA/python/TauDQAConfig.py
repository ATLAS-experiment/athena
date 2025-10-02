
#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file TauDQAConfig.py
@author N.Pettersson
@date 2022-06-16
@brief Main CA-based python configuration for TauDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValTauCfg(flags, tauContainer="TauJets",**kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("TauContainerName", tauContainer)
   
    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isMC", flags.Input.isMC)

    from TauDQA.TauDQATools import TauDQAPrimitiveTauSelectionToolCfg, TauDQANominalTauSelectionToolCfg, TauDQATauTruthMatchingToolCfg
    kwargs.setdefault("NominalTauSelectionTool", TauDQANominalTauSelectionToolCfg(flags))
    kwargs.setdefault("PrimitiveTauSelectionTool", TauDQAPrimitiveTauSelectionToolCfg(flags))
    kwargs.setdefault("TauTruthMatchingTool", TauDQATauTruthMatchingToolCfg(flags))
    tool = CompFactory.PhysValTau(name=tauContainer, **kwargs)
    acc.setPrivateTools(tool)
    return acc
