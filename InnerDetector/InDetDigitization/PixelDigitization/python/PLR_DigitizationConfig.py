"""Define methods to construct configured PLR Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

from PixelConditionsAlgorithms.PLR_ConditionsConfig import (
    PLR_ConfigCondAlgCfg, PLR_ChargeCalibCondAlgCfg,
    PLR_DistortionAlgCfg
)
from PixelConditionsTools.PLR_ConditionsSummaryConfig import PLR_ConditionsSummaryCfg
from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
from PixelReadoutGeometry.PixelReadoutGeometryConfig import PLR_ReadoutManagerCfg
from SiLorentzAngleTool.PLR_LorentzAngleConfig import PLR_LorentzAngleToolCfg
from SiPropertiesTool.PLR_SiPropertiesConfig import PLR_SiPropertiesToolCfg
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType


# The earliest and last bunch crossing times for which interactions will be sent
# to the PLR Digitization code
def PLR_FirstXing(flags):
    return -25


def PLR_LastXing(flags):
    if flags.Beam.BunchSpacing > 50:
        return 75
    else:
        return 25


def PLR_EnergyDepositionToolCfg(flags, name="PLR_EnergyDepositionTool", **kwargs):
    """Return a configured EnergyDepositionTool"""
    acc = PLR_DistortionAlgCfg(flags)
    kwargs.setdefault("DeltaRayCut", 117.)
    kwargs.setdefault("nCols", 5)
    kwargs.setdefault("LoopLimit", 100000)
    kwargs.setdefault("doBichsel", True)
    kwargs.setdefault("doBichselBetaGammaCut", 0.7) # dEdx not quite consistent below this
    kwargs.setdefault("doDeltaRay", False)          # needs validation
    kwargs.setdefault("doPU", True)
    kwargs.setdefault("PixelDistortionData", "PLR_DistortionData")
    acc.setPrivateTools(CompFactory.EnergyDepositionTool(name, **kwargs))
    return acc


def PLR_RD53SimToolCfg(flags, name="PLR_RD53SimTool", **kwargs):
    """Return a RD53SimTool configured for PLR"""
    acc = PLR_ReadoutManagerCfg(flags)
    acc.merge(PLR_ConfigCondAlgCfg(flags))
    acc.merge(PLR_ChargeCalibCondAlgCfg(flags))
    kwargs.setdefault("BarrelEC", 4)
    kwargs.setdefault("DoNoise", flags.Digitization.DoInnerDetectorNoise)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("PixelChargeCalibCondData", "PLR_ChargeCalibCondData")
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PLR_ConditionsSummaryCfg(flags)))
    kwargs.setdefault("DoTimeWalk", False) 
    acc.setPrivateTools(CompFactory.RD53SimTool(name, **kwargs))
    return acc


def PLR_SensorSimPlanarToolCfg(flags, name="PLR_SensorSimPlanarTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSimPlanarTool for PLR"""
    acc = PLR_ConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(PLR_SiPropertiesToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PLR_LorentzAngleToolCfg(flags)))
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.PixelPlanarRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", True)
    if flags.Digitization.PixelPlanarRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        # acc.merge(PLR_RadSimFluenceMapAlgCfg(flags))  # TODO: not supported yet
        pass
    acc.setPrivateTools(CompFactory.SensorSimPlanarTool(name, **kwargs))
    return acc


def PLR_SensorSim3DToolCfg(flags, name="PLR_SensorSim3DTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSim3DTool for PLR"""
    acc = PLR_ConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(PLR_SiPropertiesToolCfg(flags)))
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.Pixel3DRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", True)
    if flags.Digitization.Pixel3DRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        # acc.merge(PLR_RadSimFluenceMapAlgCfg(flags))  # TODO: not supported yet
        pass
    acc.setPrivateTools(CompFactory.SensorSim3DTool(name, **kwargs))
    return acc


def PLR_DigitizationBasicToolCfg(flags, name="PLR_DigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with configured PixelDigitizationTool for PLR"""
    acc = PLR_ReadoutGeometryCfg(flags)
    # set up tool handle lists
    chargeTools = []
    feSimTools = []
    chargeTools.append(acc.popToolsAndMerge(PLR_SensorSimPlanarToolCfg(flags)))
    # chargeTools.append(acc.popToolsAndMerge(PLR_SensorSim3DToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(PLR_RD53SimToolCfg(flags)))
    kwargs.setdefault("PixelIDName", "PLR_ID")
    kwargs.setdefault("ChargeTools", chargeTools)
    kwargs.setdefault("EnergyDepositionTool", acc.popToolsAndMerge(PLR_EnergyDepositionToolCfg(flags)))
    kwargs.setdefault("FrontEndSimTools", feSimTools)
    kwargs.setdefault("InputObjectName", "PLR_Hits") 
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", PLR_FirstXing(flags))
        kwargs.setdefault("LastXing", PLR_LastXing(flags))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.PixelDigitizationTool(name, **kwargs, ))
    return acc


def PLR_DigitizationToolCfg(flags, name="PLR_DigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured PLR_DigitizationBasicTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(PLR_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", "")
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("RDOCollName", flags.Overlay.BkgPrefix + "PLR_RDOs")
        kwargs.setdefault("SDOCollName", flags.Overlay.BkgPrefix + "PLR_SDO_Map")
    else:
        kwargs.setdefault("RDOCollName", "PLR_RDOs")
        kwargs.setdefault("SDOCollName", "PLR_SDO_Map")
    tool = acc.popToolsAndMerge(PLR_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PLR_DigitizationHSToolCfg(flags, name="PLR_DigitizationHSTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for Hard Scatter"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PLR_RangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    kwargs.setdefault("HardScatterSplittingMode", 1)
    tool = acc.popToolsAndMerge(PLR_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PLR_DigitizationPUToolCfg(flags, name="PLR_DigitizationPUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUp"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PLR_RangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    kwargs.setdefault("HardScatterSplittingMode", 2)
    kwargs.setdefault("RDOCollName", "PLR_PU_RDOs")
    kwargs.setdefault("SDOCollName", "PLR_PU_SDO_Map")
    tool = acc.popToolsAndMerge(PLR_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PLR_DigitizationSplitNoMergePUToolCfg(flags, name="PLR_DigitizationSplitNoMergePUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUpPLR_Hits"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PLR_RangeCfg(flags))
    acc.merge(PileUpMergeSvcCfg(flags, Intervals=rangetool))
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("InputObjectName", "PileupPLR_Hits")
    kwargs.setdefault("RDOCollName", "PLR_PU_RDOs")
    kwargs.setdefault("SDOCollName", "PLR_PU_SDO_Map")
    tool = acc.popToolsAndMerge(PLR_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PLR_OverlayDigitizationToolCfg(flags, name="PLR_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("RDOCollName", flags.Overlay.SigPrefix + "PLR_RDOs")
    kwargs.setdefault("SDOCollName", flags.Overlay.SigPrefix + "PLR_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return PLR_DigitizationBasicToolCfg(flags, name, **kwargs)


def PLR_RangeCfg(flags, name="PLR_Range", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", PLR_FirstXing(flags))
    kwargs.setdefault("LastXing", PLR_LastXing(flags))
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#PLR_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def PLR_OutputCfg(flags):
    """Return ComponentAccumulator with Output for PLR. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["PixelRDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def PLR_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for PLR digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(PLR_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def PLR_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with PLR Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["SiHitCollection#PLR_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(PLR_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)
    acc.addEventAlgo(CompFactory.PixelDigitization(name="PLR_OverlayDigitization", **kwargs))
    return acc


# with output defaults
def PLR_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for PLR digitization and Output"""
    acc = PLR_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(PLR_OutputCfg(flags))
    return acc


# additional specialisations
def PLR_DigitizationHSCfg(flags, **kwargs):
    """Return ComponentAccumulator for Hard-Scatter-only PLR digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(PLR_DigitizationHSToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(PLR_OutputCfg(flags))
    return acc


def PLR_DigitizationPUCfg(flags, **kwargs):
    """Return ComponentAccumulator with Pile-up-only PLR digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(PLR_DigitizationPUToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(PLR_OutputCfg(flags))
    return acc
