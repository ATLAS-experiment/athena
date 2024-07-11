"""Define methods to construct configured TGC Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import TgcDigitToTgcRDOCfg
from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the TgcDigitizationTool.
def TGC_FirstXing():
    return -50


def TGC_LastXing():
    return 75


def TGC_RangeCfg(flags, name="TGC_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for TGC"""
    kwargs.setdefault("FirstXing", TGC_FirstXing())
    kwargs.setdefault("LastXing", TGC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    if flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("ItemList", ["xAOD::MuonSimHitContainer#xTgcSimHits",
                                       "xAOD::MuonSimHitAuxContainer#xTgcSimHitsAux."])
    else:
        kwargs.setdefault("ItemList", ["TGCSimHitCollection#TGC_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def TGC_DigitizationToolCfg(flags, name="TgcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured TgcDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(TGC_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", TGC_FirstXing())
        kwargs.setdefault("LastXing", TGC_LastXing())
    
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "TGC_SDO")
        kwargs.setdefault("OutputObjectName", flags.Overlay.BkgPrefix +"TGC_DIGITS")
    else:
        kwargs.setdefault("OutputSDOName", "TGC_SDO")
        kwargs.setdefault("OutputObjectName", "TGC_DIGITS")

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))

    the_tool = None
    if not flags.Muon.usePhaseIIGeoSetup:
        from MuonConfig.MuonCondAlgConfig import TgcDigitCondAlgCfg
        acc.merge(TgcDigitCondAlgCfg(flags))
        kwargs.setdefault("TGCDigitASDposKey", "TGCDigitASDposData")
        kwargs.setdefault("TGCDigitTimeOffsetKey", "TGCDigitTimeOffsetData")
        kwargs.setdefault("TGCDigitCrosstalkKey", "TGCDigitCrosstalkData")

        from AthenaConfiguration.Enums  import LHCPeriod
        kwargs.setdefault("FourBunchDigitization", flags.GeoModel.Run >= LHCPeriod.Run3)
        the_tool = CompFactory.TgcDigitizationTool(name, **kwargs)
    else:
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        acc.merge(ActsGeometryContextAlgCfg(flags))
        kwargs.setdefault("StreamName", "TgcSimForklift")
        kwargs.setdefault("SimHitKey", "xTgcSimHits")
        kwargs.setdefault("EffiDataKey", "")
        the_tool = CompFactory.MuonR4.TgcFastDigiTool(name, **kwargs)
        
    acc.setPrivateTools(the_tool)
    return acc


def TGC_OverlayDigitizationToolCfg(flags, name="Tgc_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with TgcDigitizationTool configured for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "TGC_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "TGC_SDO")
    the_tool = acc.popToolsAndMerge(TGC_DigitizationToolCfg(flags,name=name, **kwargs))
    acc.setPrivateTools(the_tool)
    return acc


def TGC_OutputCfg(flags):
    """Return ComponentAccumulator with Output for TGC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["TgcRdoContainer#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            ItemList += ["xAOD::MuonSimHitContainer#*TGC_SDO",
                        "xAOD::MuonSimHitAuxContainer#*TGC_SDOAux."]

            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def TGC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for TGC digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(TGC_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def TGC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with TGC Overlay digitization"""
    acc = MuonGeoModelCfg(flags)
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        if flags.Muon.usePhaseIIGeoSetup:
            acc.merge(SGInputLoaderCfg(flags,["xAOD::MuonSimHitContainer#xTgcSimHits",
                                              "xAOD::MuonSimHitAuxContainer#xTgcSimHitsAux."]))
        else:
            acc.merge(SGInputLoaderCfg(flags, ["TGCSimHitCollection#TGC_Hits"]))

    kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(TGC_OverlayDigitizationToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    the_alg = CompFactory.MuonDigitizer(name="TGC_OverlayDigitizer", **kwargs)
    acc.addEventAlgo(the_alg)
    return acc


# with output defaults
def TGC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for TGC digitization and Output"""
    acc = TGC_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(TGC_OutputCfg(flags))
    return acc


def TGC_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with TGC digitization and Digit to TGCRDO"""
    acc = TGC_DigitizationCfg(flags)
    acc.merge(TGCCablingConfigCfg(flags))
    acc.merge(TgcDigitToTgcRDOCfg(flags))
    return acc
