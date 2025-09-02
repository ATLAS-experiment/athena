#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def DiTauDQANominalDiTauSelectionToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("ConfigPath", "")
    from ROOT import TauAnalysisTools
    selectioncuts = TauAnalysisTools.DiTauSelectionCuts
    kwargs.setdefault("SelectionCuts", int(selectioncuts.DiTauCutPt | selectioncuts.DiTauCutAbsCharge))
    kwargs.setdefault("PtMin", 50.0)
    kwargs.setdefault("AbsCharge", 0)
    from TauAnalysisTools.DiTauAnalysisToolsConfig import DiTauSelectionToolCfg
    nominalseltool = acc.popToolsAndMerge(DiTauSelectionToolCfg(flags, "NominalDiTauSelectionTool", **kwargs))

    return nominalseltool

def DiTauDQATauTruthMatchingToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    
    from TauAnalysisTools.DiTauAnalysisToolsConfig import DiTauTruthMatchingToolCfg
    matchingtool = acc.popToolsAndMerge(DiTauTruthMatchingToolCfg(flags, "DiTauTruthMatchingTool", **kwargs))

    return matchingtool



