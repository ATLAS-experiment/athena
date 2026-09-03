# 
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def PhotonSingleBDTCalculator_Cfg(
        flags, name="PhotonSingleBDTCalculator", isConv=True, useNFs=False, **kwargs):
    acc = ComponentAccumulator()

    modelFile = "ElectronPhotonSelectorTools/offline/"
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        modelFile += "mc23_20260824/" + (
            "NFs/LightGBM_model_mc23ade_v23NF_DeltaE_fix" if useNFs else
            "FudgeFactors/LightGBM_model_mc23ade_v23FF_DeltaE_fix")
    else:
        modelFile += "mc20_20260824/" + (
            "NFs/LightGBM_model_mc20ade_v15NF_DeltaE_fix" if useNFs else
            "FudgeFactors/LightGBM_model_mc20ade_v15FF_DeltaE_fix")
    modelFile += ("_converted" if isConv else "_unconverted") + ".root"

    kwargs.setdefault("ModelFile", modelFile)
    kwargs.setdefault("BDTTreeName", "lgbm")

    acc.setPrivateTools(CompFactory.PhotonIDBDT.PhotonSingleBDTCalculator(
        name + ("Conv" if isConv else "Unconv") + ("_NFs" if useNFs else ""), **kwargs))
    return acc


def AsgPhotonBDTSelectorCfg(
        flags, name="AsgPhotonBDTSelector", useNFs=False, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("ReapplyWPIfNoShowerShapes", False)

    suffix = "_NFs" if useNFs else ""
    kwargs.setdefault("WorkingPoint", "TightBDTPhoton_" +
                      ("Run3" if flags.GeoModel.Run >= LHCPeriod.Run3 else "Run2") +
                      suffix)
    kwargs.setdefault("IsEMDecoration", "BDTIsEM" + suffix)
    kwargs.setdefault("ContainerName", "Photons")
    kwargs.setdefault("ScoreDecoration", "BDTScore" + suffix)

    acc.setPrivateTools(
        CompFactory.PhotonIDBDT.AsgPhotonBDTSelector(name + suffix, **kwargs))
    return acc


def PhotonBDTCalculatorCfg(
        flags, name="PhotonBDTCalculator", useNFs=False, **kwargs):
    acc = ComponentAccumulator()

    if "ToolConv" not in kwargs:
        singleConv = acc.popToolsAndMerge(
            PhotonSingleBDTCalculator_Cfg(flags, isConv=True, useNFs=useNFs))
        acc.addPublicTool(singleConv)
        kwargs.setdefault("ToolConv", singleConv)

    if "ToolUnconv" not in kwargs:
        singleUnconv = acc.popToolsAndMerge(
            PhotonSingleBDTCalculator_Cfg(flags, isConv=False, useNFs=useNFs))
        acc.addPublicTool(singleUnconv)
        kwargs.setdefault("ToolUnconv", singleUnconv)

    suffix = ("_NFs" if useNFs else "")
    kwargs.setdefault("DecorationName", "BDTScore" + suffix)
    kwargs.setdefault("ExcludeTRT", flags.GeoModel.Run >= LHCPeriod.Run3)
    kwargs.setdefault("ReserveVarsConv", 12)
    kwargs.setdefault("ReserveVarsUnconv", 12)

    acc.setPrivateTools(CompFactory.PhotonIDBDT.PhotonBDTCalculator(
        name + suffix, **kwargs))
    return acc
