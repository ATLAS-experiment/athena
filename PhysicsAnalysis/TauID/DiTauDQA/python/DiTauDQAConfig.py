
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''@file DiTauDQAConfig.py
@author N.Pettersson
@author A.DeMaria
@brief Main CA-based python configuration for DiTauDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValDiTauCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DiTauContainerName", "DiTauJetsLowPt")
   
    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isMC", flags.Input.isMC)

    from DiTauDQA.DiTauDQATools import DiTauDQANominalDiTauSelectionToolCfg
    kwargs.setdefault("NominalDiTauSelectionTool", DiTauDQANominalDiTauSelectionToolCfg(flags))

    tool = CompFactory.PhysValDiTau(name=kwargs["DiTauContainerName"], **kwargs)
    acc.setPrivateTools(tool)
    return acc

