# 
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


# Default configurations
# separate defaults for Run 2 and Run 3, and for using NFs or using fudge factors
def getPhotonBDTConfig(flags, useNFs):

    from AthenaConfiguration.Enums import LHCPeriod

    isRun3 = (flags.GeoModel.Run == LHCPeriod.Run3)

    if isRun3:
        if useNFs:
            return dict(
                workingPoint="TightBDTPhoton_Run3_NFs",
                modelConv="ElectronPhotonSelectorTools/offline/mc23_20260310/NFs/LightGBM_model_mc23ade_v23NF_converted.root",
                modelUnconv="ElectronPhotonSelectorTools/offline/mc23_20260310/NFs/LightGBM_model_mc23ade_v23NF_unconverted.root",
                scoreDecoration="BDTScore_NFs",
                isEMDecoration="BDTIsEM_NFs",
            )
        else:
            return dict(
                workingPoint="TightBDTPhoton_Run3",
                modelConv="ElectronPhotonSelectorTools/offline/mc23_20260310/FudgeFactors/LightGBM_model_mc23ade_v23_converted.root",
                modelUnconv="ElectronPhotonSelectorTools/offline/mc23_20260310/FudgeFactors/LightGBM_model_mc23ade_v23_unconverted.root",
                scoreDecoration="BDTScore",
                isEMDecoration="BDTIsEM",
            )

    else:
        if useNFs:
            return dict(
                workingPoint="TightBDTPhoton_Run2_NFs",
                modelConv="ElectronPhotonSelectorTools/offline/mc20_20260310/NFs/LightGBM_model_mc20ade_v15NF_converted.root",
                modelUnconv="ElectronPhotonSelectorTools/offline/mc20_20260310/NFs/LightGBM_model_mc20ade_v15NF_unconverted.root",
                scoreDecoration="BDTScore_NFs",
                isEMDecoration="BDTIsEM_NFs",
            )
        else:
            return dict(
                workingPoint="TightBDTPhoton_Run2",
                modelConv="ElectronPhotonSelectorTools/offline/mc20_20260310/FudgeFactors/LightGBM_model_mc20ade_v15_converted.root",
                modelUnconv="ElectronPhotonSelectorTools/offline/mc20_20260310/FudgeFactors/LightGBM_model_mc20ade_v15_unconverted.root",
                scoreDecoration="BDTScore",
                isEMDecoration="BDTIsEM",
            )
        
def AsgPhotonBDTSelectorCfg(flags,
                           name="AsgPhotonBDTSelector",
                           workingPoint=None,
                           useNFs=False,
                           computeIfMissing=True,
                           forceRecompute=False,
                           reapplyWPIfNoShowerShapes=False,
                           reserveVarsConv=12,
                           reserveVarsUnconv=12,
                           modelConv=None,
                           modelUnconv=None,
                           scoreDecoration=None,
                           isEMDecoration=None):

    acc = ComponentAccumulator()

    isRun3 = (flags.GeoModel.Run == LHCPeriod.Run3)
    excludeTRT = isRun3

    # Get default configurations
    defaultCfg = getPhotonBDTConfig(flags, useNFs)

    # allow override of default configurations
    workingPoint = workingPoint or defaultCfg["workingPoint"]
    modelConv = modelConv or defaultCfg["modelConv"]
    modelUnconv = modelUnconv or defaultCfg["modelUnconv"]
    scoreDecoration = scoreDecoration or defaultCfg["scoreDecoration"]
    isEMDecoration = isEMDecoration or defaultCfg["isEMDecoration"]

    Single = CompFactory.PhotonIDBDT.PhotonSingleBDTCalculator
    Calc   = CompFactory.PhotonIDBDT.PhotonBDTCalculator
    Sel    = CompFactory.PhotonIDBDT.AsgPhotonBDTSelector

    singleConv = Single(f"{name}_SingleConv",
                        ModelFile=modelConv,
                        BDTTreeName="lgbm")
    acc.addPublicTool(singleConv)

    singleUnconv = Single(f"{name}_SingleUnconv",
                          ModelFile=modelUnconv,
                          BDTTreeName="lgbm")
    acc.addPublicTool(singleUnconv)

    bdtCalc = Calc(f"{name}_BDTCalc",
                   ToolConv=singleConv,
                   ToolUnconv=singleUnconv,
                   DecorationName=scoreDecoration,
                   ExcludeTRT=excludeTRT,
                   ForceRecompute=forceRecompute,
                   ReserveVarsConv=reserveVarsConv,
                   ReserveVarsUnconv=reserveVarsUnconv)
    acc.addPublicTool(bdtCalc)

    selector = Sel(name,
                   WorkingPoint=workingPoint,
                   ScoreDecoration=scoreDecoration,
                   ComputeIfMissing=computeIfMissing,
                   ReapplyWPIfNoShowerShapes=reapplyWPIfNoShowerShapes,
                   IsEMDecoration=isEMDecoration,
                   ExcludeTRT=excludeTRT,
                   BDTTool=bdtCalc)
    acc.setPrivateTools(selector)

    return acc

def PhotonBDTCalculatorCfg(flags,
                           name="PhotonBDTCalculator",
                           useNFs=False,
                           forceRecompute=False,
                           modelConv=None,
                           modelUnconv=None,
                           scoreDecoration=None,
                           reserveVarsConv=12,
                           reserveVarsUnconv=12):

    acc = ComponentAccumulator()

    # get default configurations
    defaultCfg = getPhotonBDTConfig(flags, useNFs)

    # allow override of default configurations
    modelConv = modelConv or defaultCfg["modelConv"]
    modelUnconv = modelUnconv or defaultCfg["modelUnconv"]
    scoreDecoration = scoreDecoration or defaultCfg["scoreDecoration"]

    Single = CompFactory.PhotonIDBDT.PhotonSingleBDTCalculator
    Calc   = CompFactory.PhotonIDBDT.PhotonBDTCalculator

    singleConv = Single(f"{name}_SingleConv",
                        ModelFile=modelConv,
                        BDTTreeName="lgbm")
    acc.addPublicTool(singleConv)

    singleUnconv = Single(f"{name}_SingleUnconv",
                          ModelFile=modelUnconv,
                          BDTTreeName="lgbm")
    acc.addPublicTool(singleUnconv)

    bdtCalc = Calc(name,
                   ToolConv=singleConv,
                   ToolUnconv=singleUnconv,
                   DecorationName=scoreDecoration,
                   ExcludeTRT=(flags.GeoModel.Run == LHCPeriod.Run3),
                   ForceRecompute=forceRecompute,
                   ReserveVarsConv=reserveVarsConv,
                   ReserveVarsUnconv=reserveVarsUnconv)
    acc.setPrivateTools(bdtCalc)

    return acc