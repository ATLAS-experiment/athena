"""Define methods to construct configured MM Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
from MuonConfig.MuonByteStreamCnvTestConfig import MM_DigitToRDOCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

# The earliest and last bunch crossing times for which interactions will be sent
# to the MMDigitizationTool.
def MM_FirstXing():
    return -200


def MM_LastXing():
    return 200


def MM_RangeCfg(flags, name="MMRange", **kwargs):
    """Return a PileUpXingFolder tool configured for MM"""
    kwargs.setdefault("FirstXing", MM_FirstXing())
    kwargs.setdefault("LastXing", MM_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    if flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("ItemList", ["xAOD::MuonSimHitContainer#xMmSimHits",
                                       "xAOD::MuonSimHitAuxContainer#xMmSimHitsAux."])
    else:
        kwargs.setdefault("ItemList", ["MMSimHitCollection#MM_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def MM_DigitizationToolCfg(flags, name="MM_DigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured MM_DigitizationTool"""
    result = ComponentAccumulator()
    
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    
    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", MM_FirstXing())
            kwargs.setdefault("LastXing", MM_LastXing())
        else:
            intervals += [result.popToolsAndMerge(MM_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", result.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    else:
        kwargs.setdefault("PileUpMergeSvc", '')

    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)

    kwargs.setdefault("OutputObjectName", "MM_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "MM_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "MM_SDO")

    the_tool = None
    if not flags.Muon.usePhaseIIGeoSetup:
        from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg, MMCalibSmearingToolCfg  
        kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))
        kwargs.setdefault("SmearingTool", result.popToolsAndMerge(MMCalibSmearingToolCfg(flags)))
        result.merge(AtlasFieldCacheCondAlgCfg(flags))   
        kwargs.setdefault("CheckSimHits", True)
        the_tool = CompFactory.MM_DigitizationTool(name, **kwargs)
    elif flags.Muon.doFastMMDigitization:
        kwargs.setdefault("StreamName", "MmSimForklift")
        kwargs.setdefault("SimHitKey", "xMmSimHits")    
        """
        from MuonConfig.MuonCondAlgConfig import MmDigitEffiCondAlgCfg
        result.merge(MmDigitEffiCondAlgCfg(flags,readFromJSON="EffMapMM.json"))
        kwargs.setdefault("EffiDataKey", "MmDigitEff")
        """
    
        kwargs.setdefault("EffiDataKey", "")
    
        from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
        result.merge(NswErrorCalibDbAlgCfg(flags))
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        result.merge(ActsGeometryContextAlgCfg(flags))
        the_tool = CompFactory.MuonR4.MmFastDigiTool(name, **kwargs)
    else:
        from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg, MMCalibSmearingToolCfg  
        kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))
        kwargs.setdefault("SmearingTool", result.popToolsAndMerge(MMCalibSmearingToolCfg(flags)))
        result.merge(AtlasFieldCacheCondAlgCfg(flags))   
        kwargs.setdefault("CheckSimHits", True)

        kwargs.setdefault("StreamName", "MmSimForklift")
        kwargs.setdefault("SimHitKey", "xMmSimHits")

        kwargs.setdefault("EffiDataKey", "")
        
        from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
        result.merge(NswErrorCalibDbAlgCfg(flags))
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        result.merge(ActsGeometryContextAlgCfg(flags))
        the_tool = CompFactory.MuonR4.MM_DigitizationTool(name, **kwargs)


    result.setPrivateTools(the_tool)
    return result


def MM_OverlayDigitizationToolCfg(flags, name="MM_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with MM_DigitizationTool configured for Overlay"""
    acc= ComponentAccumulator()
    kwargs.setdefault("PileUpMergeSvc", '')
   
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "MM_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "MM_SDO")
    the_tool = acc.popToolsAndMerge(MM_DigitizationToolCfg(flags, **kwargs))
    acc.setPrivateTools(the_tool)
    return acc


def MM_OutputCfg(flags):
    """Return ComponentAccumulator with Output for MM. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["Muon::MM_RawDataContainer#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            ItemList += ["xAOD::MuonSimHitContainer#*MM_SDO",
                         "xAOD::MuonSimHitAuxContainer#*MM_SDOAux."]

            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def MM_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for MM digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(MM_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def MM_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with MM Overlay digitization"""
    acc = MuonGeoModelCfg(flags)
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        if flags.Muon.usePhaseIIGeoSetup:
            acc.merge(SGInputLoaderCfg(flags,["xAOD::MuonSimHitContainer#xMmSimHits",
                                              "xAOD::muonSimHitAuxConatiner#xMmSimHitsAux."]))
        else:
            acc.merge(SGInputLoaderCfg(flags, ["MMSimHitCollection#MM_Hits"]))

    kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(MM_OverlayDigitizationToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.MuonDigitizer(name="MM_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def MM_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for MM digitization and Output"""
    acc = MM_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(MM_OutputCfg(flags))
    return acc


def MM_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with TGC digitization and Digit to TGCRDO"""
    acc = MM_DigitizationCfg(flags)
    acc.merge(MM_DigitToRDOCfg(flags))
    return acc
