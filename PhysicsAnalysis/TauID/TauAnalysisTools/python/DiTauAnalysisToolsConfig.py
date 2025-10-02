# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DiTauSelectionToolCfg(flags, name, **kwargs):
   """Configure the ditau selection tool"""
   acc = ComponentAccumulator()
   DiTauSelectionTool = CompFactory.TauAnalysisTools.DiTauSelectionTool
   acc.setPrivateTools(DiTauSelectionTool(name, **kwargs))
   return acc

def DiTauTruthMatchingToolCfg(flags, name, **kwargs):
   acc = ComponentAccumulator()
   tool = CompFactory.TauAnalysisTools.DiTauTruthMatchingTool(name, **kwargs)
   acc.setPrivateTools(tool)
   return acc

