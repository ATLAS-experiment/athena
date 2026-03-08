# 
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def AsgPhotonBDTSelectorCfg(flags,
                           name="AsgPhotonBDTSelector",
                           workingPoint="TightBDTPhoton_Run3",
                           modelConv="ElectronPhotonSelectorTools/LightGBM_model_mc23ade_v23_converted.root",
                           modelUnconv="ElectronPhotonSelectorTools/LightGBM_model_mc23ade_v23_unconverted.root",
                           bdtTreeName="lgbm",
                           scoreDecoration="BDTScore",
                           computeIfMissing=True,
                           forceRecompute=False,
                           reapplyWPIfNoShowerShapes=False,
                           isEMDecoration="BDTIsEM",
                           reserveVarsConv=12,
                           reserveVarsUnconv=12):

    acc = ComponentAccumulator()

    isRun3 = (flags.GeoModel.Run == LHCPeriod.Run3)
    excludeTRT = True if isRun3 else False

    Single = CompFactory.PhotonIDBDT.PhotonSingleBDTCalculator
    Calc   = CompFactory.PhotonIDBDT.PhotonBDTCalculator
    Sel    = CompFactory.PhotonIDBDT.AsgPhotonBDTSelector

    singleConv = Single(f"{name}_SingleConv",
                        ModelFile=modelConv,
                        BDTTreeName=bdtTreeName)

    acc.addPublicTool(singleConv)

    singleUnconv = Single(f"{name}_SingleUnconv",
                          ModelFile=modelUnconv,
                          BDTTreeName=bdtTreeName)

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