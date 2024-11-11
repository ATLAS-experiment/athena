# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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

def TauHFVetoToolCfg(flags, name, **kwargs):
   acc=ComponentAccumulator()
   execution_provider = flags.AthOnnx.ExecutionProvider
   pathToHFVetoModels = 'TauAnalysisTools/00-04-00/HFVeto'
   model_fname_bveto1p = f'{pathToHFVetoModels}/bveto1p.onnx'
   model_fname_bveto3p = f'{pathToHFVetoModels}/bveto3p.onnx'
   model_fname_cveto1p = f'{pathToHFVetoModels}/cveto1p.onnx'
   model_fname_cveto3p = f'{pathToHFVetoModels}/cveto3p.onnx'
   from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
   kwargs.setdefault("bveto1p", acc.popToolsAndMerge(
      OnnxRuntimeInferenceToolCfg(flags, model_fname_bveto1p, execution_provider, name="bveto1p")
   ))
   kwargs.setdefault("bveto3p", acc.popToolsAndMerge(
      OnnxRuntimeInferenceToolCfg(flags, model_fname_bveto3p, execution_provider, name="bveto3p")
   ))
   kwargs.setdefault("cveto1p", acc.popToolsAndMerge(
      OnnxRuntimeInferenceToolCfg(flags, model_fname_cveto1p, execution_provider, name="cveto1p")
   ))
   kwargs.setdefault("cveto3p", acc.popToolsAndMerge(
      OnnxRuntimeInferenceToolCfg(flags, model_fname_cveto3p, execution_provider, name="cveto3p")
   ))
   tool = CompFactory.TauAnalysisTools.TauHFVetoTool(name, **kwargs)
   acc.setPrivateTools(tool)
   return acc
