"""Define methods to construct configured HGTD Digitization tools and algorithms

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from HGTD_Calibration.HGTD_CalibrationConfig import HGTD_TdcCalibrationToolCfg

# The earliest bunch crossing time for which interactions will be sent
# to the HGTD Digitization code.
def HGTD_FirstXing():
    return 0
# The latest bunch crossing time for which interactions will be sent
# to the HGTD Digitization code.
def HGTD_LastXing():
    return 0
# NOTE: related to 3BC mode?


def HGTD_FrontEndToolCfg(flags, name="HGTD_FrontEndTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_FrontEndTool(name, **kwargs))
    return acc


def HGTD_SurfaceChargesGeneratorCfg(flags, name="HGTD_SurfaceChargesGenerator", **kwargs):
    acc = ComponentAccumulator()

    from HGTD_Calibration.HGTD_CalibrationConfig import HGTD_TimeResolutionToolCfg

    kwargs.setdefault("ActiveTimeWindow", 1.25)
    kwargs.setdefault("SmearMeanTime", True)
    kwargs.setdefault("TimeResolutionTool", acc.popToolsAndMerge(
        HGTD_TimeResolutionToolCfg(flags)))
    acc.setPrivateTools(CompFactory.HGTD_SurfaceChargesGenerator(name, **kwargs))
    return acc


def HGTD_DigitizationBasicToolCfg(flags, name="HGTD_DigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with configured HGTD_DigitizationTool"""
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    acc = HGTD_ReadoutGeometryCfg(flags)
    # set up tool handles
    kwargs.setdefault("FrontEnd", acc.popToolsAndMerge(HGTD_FrontEndToolCfg(flags)))
    kwargs.setdefault("SurfaceChargesGenerator", acc.popToolsAndMerge(HGTD_SurfaceChargesGeneratorCfg(flags)))
    kwargs.setdefault("HGTD_TdcCalibrationTool", acc.popToolsAndMerge(HGTD_TdcCalibrationToolCfg(flags)))
    kwargs.setdefault("InputObjectName", "HGTD_Hits")
    kwargs.setdefault("HGTDDetEleCollKey", "HGTD_DetectorElementCollection")
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", HGTD_FirstXing())
        kwargs.setdefault("LastXing", HGTD_LastXing())
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))

    acc.setPrivateTools(CompFactory.HGTD_DigitizationTool(name, **kwargs))
    return acc


def HGTD_DigitizationToolCfg(flags, name="HGTD_DigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured HGTD_DigitizationBasicTool"""
    acc = ComponentAccumulator()

    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(HGTD_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    else:
        kwargs.setdefault("MergeSvc", "")
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        if flags.HGTD.outputAltirocRDO:
            kwargs.setdefault("AltirocOutputObject", f"{flags.Overlay.BkgPrefix}HGTD_ALTIROC_RDOs")
        kwargs.setdefault("OutputObjectName", f"{flags.Overlay.BkgPrefix}HGTD_RDOs")
        kwargs.setdefault("OutputSDOName", f"{flags.Overlay.BkgPrefix}HGTD_SDO_Map")
    else:
        if flags.HGTD.outputAltirocRDO:
            kwargs.setdefault("AltirocOutputObject", "HGTD_ALTIROC_RDOs")
        kwargs.setdefault("OutputObjectName", "HGTD_RDOs")
        kwargs.setdefault("OutputSDOName", "HGTD_SDO_Map")
    pileupTool = acc.popToolsAndMerge(HGTD_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(pileupTool)
    return acc


def HGTD_OverlayDigitizationToolCfg(flags, name="HGTD_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with HGTD_DigitizationTool configured for overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputSDOName", f"{flags.Overlay.SigPrefix}HGTD_SDO_Map")
    kwargs.setdefault("MergeSvc", "")
    kwargs.setdefault("OutputObjectName", f"{flags.Overlay.SigPrefix}HGTD_RDOs")
    if flags.HGTD.outputAltirocRDO:
        kwargs.setdefault("AltirocOutputObject", f"{flags.Overlay.SigPrefix}HGTD_ALTIROC_RDOs")
    return HGTD_DigitizationBasicToolCfg(flags, name, **kwargs)


def HGTD_RangeCfg(flags, name="HGTD_Range", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", HGTD_FirstXing())
    kwargs.setdefault("LastXing", HGTD_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#HGTD_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def HGTD_OutputCfg(flags):
    """Return ComponentAccumulator with Output for HGTD. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["HGTD_RDO_Container#*"]
        if flags.HGTD.outputAltirocRDO:
            ItemList += ["HGTD_ALTIROC_RDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def HGTD_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for HGTD digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        kwargs["PileUpTools"] = acc.popToolsAndMerge(HGTD_DigitizationToolCfg(flags))
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def HGTD_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with HGTD Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["SiHitCollection#HGTD_Hits"]))

    if "DigitizationTool" not in kwargs:
        kwargs["DigitizationTool"] = acc.popToolsAndMerge(HGTD_OverlayDigitizationToolCfg(flags))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.HGTD_Digitization(name="HGTD_OverlayDigitization", **kwargs))
    return acc


# with output defaults
def HGTD_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for HGTD digitization and Output"""
    acc = HGTD_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(HGTD_OutputCfg(flags))
    return acc
