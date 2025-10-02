# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_DriftFunctionTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def TRT_DriftFunctionToolCfg(flags, name = "InDetTRT_DriftFunctionTool", **kwargs):
    acc = ComponentAccumulator()

    # Calibration DB Service
    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(TRT_CalDbToolCfg(flags)))

    # --- set Data/MC flag
    kwargs.setdefault("IsMC", flags.Input.isMC)

    # --- set HT corrections
    kwargs.setdefault("HTCorrectionBarrelXe", 1.5205)
    kwargs.setdefault("HTCorrectionEndcapXe", 1.2712)
    kwargs.setdefault("HTCorrectionBarrelAr", 1.5205)
    kwargs.setdefault("HTCorrectionEndcapAr", 1.2712)
    # --- set ToT corrections
    ToTCorrectionsBarrel = [       0.,  4.358121,  3.032195,  1.631892,  0.7408397,
                            -0.004113, -0.613288,  -0.73758, -0.623346,  -0.561229,
                             -0.29828,  -0.21344, -0.322892, -0.386718,  -0.534751,
                            -0.874178, -1.231799, -1.503689, -1.896464,  -2.385958]
    ToTCorrectionsEndcap = [       0.,  5.514777,  3.342712,  2.056626, 1.08293693,
                            0.3907979, -0.082819, -0.457485, -0.599706,  -0.427493,
                            -0.328962, -0.403399, -0.663656, -1.029428,   -1.46008,
                            -1.919092, -2.151582, -2.285481, -2.036822,   -2.15805]
    kwargs.setdefault("ToTCorrectionsBarrelXe", ToTCorrectionsBarrel)
    kwargs.setdefault("ToTCorrectionsEndcapXe", ToTCorrectionsEndcap)

    if flags.InDet.doTRTArToTCorr:
        kwargs.setdefault("ToTCorrectionsBarrelAr", ToTCorrectionsBarrel)
        kwargs.setdefault("ToTCorrectionsEndcapAr", ToTCorrectionsEndcap)
    else:
        NullToTCorrections = [ 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0. ]
        kwargs.setdefault("ToTCorrectionsBarrelAr", NullToTCorrections)
        kwargs.setdefault("ToTCorrectionsEndcapAr", NullToTCorrections)

    # Second calibration DB Service in case pile-up and physics hits have different calibrations for data overlay
    if flags.Common.ProductionStep is not ProductionStep.MinbiasPreprocessing and flags.Overlay.DataOverlay:
        if "TRTCalDbTool2" not in kwargs:
            from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_MCCalDbToolCfg
            kwargs.setdefault("TRTCalDbTool2", acc.popToolsAndMerge(TRT_MCCalDbToolCfg(flags)))

        kwargs.setdefault("IsDataOverlay", True)

    acc.setPrivateTools(CompFactory.TRT_DriftFunctionTool(name, **kwargs))
    return acc

def TRT_NoTime_DriftFunctionToolCfg(flags, name = "InDetTRT_NoTime_DriftFunctionTool", **kwargs):
    kwargs.setdefault("DummyMode", True)
    kwargs.setdefault("UniversalError", 1.15)
    return TRT_DriftFunctionToolCfg(flags, name, **kwargs)
    
def TRT_Phase_DriftFunctionToolCfg(flags, name = "InDetTRT_Phase_DriftFunctionTool", **kwargs):
    kwargs.setdefault("T0BarrelShift", False)
    return TRT_DriftFunctionToolCfg(flags, name, **kwargs)
