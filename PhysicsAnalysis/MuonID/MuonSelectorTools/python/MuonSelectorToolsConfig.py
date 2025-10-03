# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
  
#********************************************************************
# MuonsSelectionToolConfig.py 
# Configures muon selection tool which is used to select muons 
# for use in physics analysis
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def MuonSelectionToolCfg(flags, name="MuonSelectionTool", **kwargs):
    """Configure the muon selection tool"""
    acc = ComponentAccumulator()

    # Configure the Onnx tool FIRST
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg

    model_fname = "MuonSelectorTools/TightNN_Experimental_18062025/model_DNN3norm_MC20ade.onnx"
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        model_fname = "MuonSelectorTools/TightNN_Experimental_18062025/model_DNN3norm_MC23ad.onnx"

    execution_provider = OnnxRuntimeType.CPU
    # Set defaults AFTER ort_tool is available
    kwargs.setdefault("IsRun3Geo", flags.GeoModel.Run >= LHCPeriod.Run3)
    kwargs.setdefault("DisablePtCuts", True)
    kwargs.setdefault("TurnOffMomCorr", True)
    kwargs.setdefault("ORTInferenceTool", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, model_fname, execution_provider, name=name+"_ORTInferenceTool")
    ))


    # Now construct the tool with all kwargs set
    the_tool = CompFactory.CP.MuonSelectionTool(name, **kwargs)
    acc.setPrivateTools(the_tool)
    acc.printConfig(withDetails=True)

    return acc
 

