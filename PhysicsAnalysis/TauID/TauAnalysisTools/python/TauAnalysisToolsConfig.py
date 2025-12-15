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


def TauHFVetoToolCfg(flags, name, **kwargs):
   acc=ComponentAccumulator()
   execution_provider = flags.AthOnnx.ExecutionProvider
   pathToHFVetoModels = 'TauAnalysisTools/00-04-00/HFVeto'
   from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
   for model in ("bveto1p", "bveto3p", "cveto1p", "cveto3p"):
       kwargs.setdefault(model, acc.popToolsAndMerge(
           OnnxRuntimeInferenceToolCfg(flags, f'{pathToHFVetoModels}/{model}.onnx', execution_provider, name=model)
           ))
   tool = CompFactory.TauAnalysisTools.TauHFVetoTool(name, **kwargs)
   acc.setPrivateTools(tool)
   return acc

