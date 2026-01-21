# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TauSelectionToolCfg(flags, name, **kwargs):
   """Configure the tau selection tool"""
   acc = ComponentAccumulator()
   TauSelectionTool = CompFactory.TauAnalysisTools.TauSelectionTool
   acc.setPrivateTools(TauSelectionTool(name, **kwargs))
   return acc


def TauTruthMatchingToolCfg(flags, name, **kwargs):
   acc = ComponentAccumulator()
   tool = CompFactory.TauAnalysisTools.TauTruthMatchingTool(name, **kwargs)
   acc.setPrivateTools(tool)
   return acc


def BuildTruthTausCfg(flags, name, **kwargs):
    """Configure the BuildTruthTaus tool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.TauAnalysisTools.BuildTruthTaus(name, **kwargs))
    return acc


