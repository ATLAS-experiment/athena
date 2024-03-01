"""Define methods to construct configured Pixel Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from PixelConditionsAlgorithms.PixelConditionsConfig import (
    PixelConfigCondAlgCfg, PixelChargeCalibCondCfg,
    PixelDistortionAlgCfg, PixelRadSimFluenceMapAlgCfg
)
from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
from SiPropertiesTool.PixelSiPropertiesConfig import PixelSiPropertiesToolCfg
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
from AthenaConfiguration.Enums import BeamType
# The earliest and last bunch crossing times for which interactions will be sent
# to the Pixel Digitization code
def Pixel_FirstXing(flags):
    if flags.Beam.estimatedLuminosity > 0.5e33:
        return -25
    else:
        return -50


def Pixel_LastXing(flags):
    if flags.Beam.estimatedLuminosity > 0.5e33:
        if flags.Beam.BunchSpacing > 50:
            return 75
        else:
            return 25
    else:
        return 100


def RadDamageUtilCfg(flags, name="RadDamageUtil", **kwargs):
    """Return a configured RadDamageUtil"""
    kwargs.setdefault("defaultRamo", 1)
    kwargs.setdefault("betaElectrons", 4.5e-16)
    kwargs.setdefault("betaHoles", 6.0e-16)
    kwargs.setdefault("saveDebugMaps", False)
    RadDamageUtil = CompFactory.RadDamageUtil
    return RadDamageUtil(name, **kwargs)


def EfieldInterpolatorCfg(flags, name="EfieldInterpolator", **kwargs):
    """Return a configured EfieldInterpolator"""
    kwargs.setdefault("initialized", False)
    kwargs.setdefault("useSpline", True)
    kwargs.setdefault("sensorDepth", 200)
    EfieldInterpolator = CompFactory.EfieldInterpolator
    return EfieldInterpolator(name, **kwargs)


def EnergyDepositionToolCfg(flags, name="EnergyDepositionTool", **kwargs):
    """Return a configured EnergyDepositionTool"""
    acc = PixelDistortionAlgCfg(flags)
    kwargs.setdefault("DeltaRayCut", 80.7687)
    kwargs.setdefault("nCols", 5)
    kwargs.setdefault("LoopLimit", 100000)
    kwargs.setdefault("doBichsel", True)
    kwargs.setdefault("doBichselBetaGammaCut", 0.7) # dEdx not quite consistent below this
    kwargs.setdefault("doDeltaRay", False)          # needs validation
    kwargs.setdefault("doPU", False)
    EnergyDepositionTool = CompFactory.EnergyDepositionTool
    acc.setPrivateTools(EnergyDepositionTool(name, **kwargs))
    return acc


def SensorSimPlanarToolCfg(flags, name="SensorSimPlanarTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSimPlanarTool"""
    acc = PixelConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(PixelSiPropertiesToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags)))
    SensorSimPlanarTool = CompFactory.SensorSimPlanarTool
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.PixelPlanarRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", False)
    if flags.Digitization.PixelPlanarRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        acc.merge(PixelRadSimFluenceMapAlgCfg(flags))
    acc.setPrivateTools(SensorSimPlanarTool(name, **kwargs))
    return acc


def SensorSim3DToolCfg(flags, name="SensorSim3DTool", **kwargs):
    """Return ComponentAccumulator with configured SensorSim3DTool"""
    acc = PixelConfigCondAlgCfg(flags)
    kwargs.setdefault("SiPropertiesTool", acc.popToolsAndMerge(PixelSiPropertiesToolCfg(flags)))
    SensorSim3DTool = CompFactory.SensorSim3DTool
    kwargs.setdefault("RadiationDamageSimulationType", flags.Digitization.Pixel3DRadiationDamageSimulationType.value)
    kwargs.setdefault("IsITk", False)
    if flags.Digitization.Pixel3DRadiationDamageSimulationType is not PixelRadiationDamageSimulationType.NoRadiationDamage:
        acc.merge(PixelRadSimFluenceMapAlgCfg(flags))
    acc.setPrivateTools(SensorSim3DTool(name, **kwargs))
    return acc


def BarrelFEI4SimToolCfg(flags, name="BarrelFEI4SimTool", **kwargs):
    """Return a FEI4SimTool configured for Barrel"""
    acc = PixelReadoutManagerCfg(flags)
    acc.merge(PixelConfigCondAlgCfg(flags))
    acc.merge(PixelChargeCalibCondCfg(flags))
    kwargs.setdefault("BarrelEC", 0)
    kwargs.setdefault("DoNoise", flags.Digitization.DoInnerDetectorNoise)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    FEI4SimTool = CompFactory.FEI4SimTool
    acc.setPrivateTools(FEI4SimTool(name, **kwargs))
    return acc


def DBMFEI4SimToolCfg(flags, name="DBMFEI4SimTool", **kwargs):
    """Return a FEI4SimTool configured for Endcap"""
    acc = PixelReadoutManagerCfg(flags)
    acc.merge(PixelConfigCondAlgCfg(flags))
    acc.merge(PixelChargeCalibCondCfg(flags))
    kwargs.setdefault("BarrelEC", 4)
    kwargs.setdefault("DoNoise", flags.Digitization.DoInnerDetectorNoise)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    FEI4SimTool = CompFactory.FEI4SimTool
    acc.setPrivateTools(FEI4SimTool(name, **kwargs))
    return acc


def BarrelFEI3SimToolCfg(flags, name="BarrelFEI3SimTool", **kwargs):
    """Return a FEI3SimTool configured for Barrel"""
    acc = PixelReadoutManagerCfg(flags)
    acc.merge(PixelConfigCondAlgCfg(flags))
    acc.merge(PixelChargeCalibCondCfg(flags))
    kwargs.setdefault("BarrelEC", 0)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    kwargs.setdefault("HitDuplication", (flags.GeoModel.Run is LHCPeriod.Run1))
    FEI3SimTool = CompFactory.FEI3SimTool
    acc.setPrivateTools(FEI3SimTool(name, **kwargs))
    return acc


def EndcapFEI3SimToolCfg(flags, name="EndcapFEI3SimTool", **kwargs):
    """Return a FEI3SimTool configured for Endcap"""
    acc = PixelReadoutManagerCfg(flags)
    acc.merge(PixelConfigCondAlgCfg(flags))
    acc.merge(PixelChargeCalibCondCfg(flags))
    kwargs.setdefault("BarrelEC", 2)
    kwargs.setdefault("PixelReadoutManager", acc.getPrimary())
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
    kwargs.setdefault("HitDuplication", (flags.GeoModel.Run is LHCPeriod.Run1))
    FEI3SimTool = CompFactory.FEI3SimTool
    acc.setPrivateTools(FEI3SimTool(name, **kwargs))
    return acc


def PixelDigitizationBasicToolCfg(flags, name="PixelDigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with configured PixelDigitizationTool"""
    acc = PixelReadoutGeometryCfg(flags)
    # set up tool handle lists
    chargeTools = []
    feSimTools = []
    chargeTools.append(acc.popToolsAndMerge(SensorSimPlanarToolCfg(flags)))
    chargeTools.append(acc.popToolsAndMerge(SensorSim3DToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(BarrelFEI4SimToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(DBMFEI4SimToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(BarrelFEI3SimToolCfg(flags)))
    feSimTools.append(acc.popToolsAndMerge(EndcapFEI3SimToolCfg(flags)))
    kwargs.setdefault("InputObjectName", "PixelHits")
    kwargs.setdefault("ChargeTools", chargeTools)
    kwargs.setdefault("FrontEndSimTools", feSimTools)
    kwargs.setdefault("EnergyDepositionTool", acc.popToolsAndMerge(EnergyDepositionToolCfg(flags)))
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", Pixel_FirstXing(flags))
        kwargs.setdefault("LastXing", Pixel_LastXing(flags))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    PixelDigitizationTool = CompFactory.PixelDigitizationTool
    acc.setPrivateTools(PixelDigitizationTool(name, **kwargs))
    return acc


def PixelDigitizationToolCfg(flags, name="PixelDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured BasicPixelDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(PixelRangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("RDOCollName", flags.Overlay.BkgPrefix + "PixelRDOs")
        kwargs.setdefault("SDOCollName", flags.Overlay.BkgPrefix + "PixelSDO_Map")
    else:
        kwargs.setdefault("RDOCollName", "PixelRDOs")
        kwargs.setdefault("SDOCollName", "PixelSDO_Map")
    tool = acc.popToolsAndMerge(PixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PixelGeantinoTruthDigitizationToolCfg(flags, name="PixelGeantinoTruthDigitizationTool", **kwargs):
    """Return configured PixelDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(PixelRangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("VetoPileUpTruthLinks", False)
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    PixelDigitizationTool = CompFactory.PixelDigitizationTool
    acc.setPrivateTools(PixelDigitizationTool(name, **kwargs))
    return acc


def PixelDigitizationHSToolCfg(flags, name="PixelDigitizationHSTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for Hard Scatter"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 1)
    tool = acc.popToolsAndMerge(PixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PixelDigitizationPUToolCfg(flags, name="PixelDigitizationPUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUp"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 2)
    kwargs.setdefault("RDOCollName", "Pixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "Pixel_PU_SDO_Map")
    tool = acc.popToolsAndMerge(PixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PixelDigitizationSplitNoMergePUToolCfg(flags, name="PixelDigitizationSplitNoMergePUTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for PileUpPixelHits"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(PixelRangeCfg(flags))
    kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("InputObjectName", "PileupPixelHits")
    kwargs.setdefault("RDOCollName", "Pixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "Pixel_PU_SDO_Map")
    tool = acc.popToolsAndMerge(PixelDigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def PixelOverlayDigitizationToolCfg(flags, name="PixelOverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with PixelDigitizationTool configured for overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("RDOCollName", flags.Overlay.SigPrefix + "PixelRDOs")
    kwargs.setdefault("SDOCollName", flags.Overlay.SigPrefix + "PixelSDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("PileUpMergeSvc", '')
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)


def PixelRangeCfg(flags, name="PixelRange", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", Pixel_FirstXing(flags))
    kwargs.setdefault("LastXing", Pixel_LastXing(flags))
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#PixelHits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def PixelOutputCfg(flags):
    """Return ComponentAccumulator with Output for Pixel. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["PixelRDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        if flags.InDet.savePixelSiHits:
            ItemList += ["SiHitCollection#PixelHits"]
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def PixelDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for Pixel digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(PixelDigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def PixelOverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with Pixel Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["SiHitCollection#PixelHits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(PixelOverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)
    PixelDigitization = CompFactory.PixelDigitization
    acc.addEventAlgo(PixelDigitization(name="PixelOverlayDigitization", **kwargs))
    return acc


# with output defaults
def PixelDigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for Pixel digitization and Output"""
    acc = PixelDigitizationBasicCfg(flags, **kwargs)
    acc.merge(PixelOutputCfg(flags))
    return acc


# additional specialisations
def PixelDigitizationHSCfg(flags, **kwargs):
    """Return ComponentAccumulator for Hard-Scatter-only Pixel digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(PixelDigitizationHSToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(PixelOutputCfg(flags))
    return acc


def PixelDigitizationPUCfg(flags, **kwargs):
    """Return ComponentAccumulator with Pile-up-only Pixel digitization and Output"""
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(PixelDigitizationPUToolCfg(flags))
    kwargs["PileUpTools"] = tool
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    acc.merge(PixelOutputCfg(flags))
    return acc
