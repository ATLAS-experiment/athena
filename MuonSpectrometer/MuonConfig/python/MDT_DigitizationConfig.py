"""Define methods to construct configured MDT Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import MdtDigitToMdtRDOCfg
from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
from DigitizationConfig.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the MdtDigitizationTool.
def MDT_FirstXing():
    return -800


def MDT_LastXing():
    # was 800 for large time window
    return 150


def MDT_RangeCfg(flags, name="MDT_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for MDT"""
    kwargs.setdefault("FirstXing", MDT_FirstXing())
    kwargs.setdefault("LastXing",  MDT_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    if flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("ItemList", ["xAOD::MuonSimHitContainer#xMdtSimHits",
                                       "xAOD::MuonSimHitAuxContainer#xMdtSimHitsAux."])
    else:        
        kwargs.setdefault("ItemList", ["MDTSimHitCollection#MDT_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def RT_Relation_DB_DigiToolCfg(flags, name="RT_Relation_DB_DigiTool", **kwargs):
    """Return an RT_Relation_DB_DigiTool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.RT_Relation_DB_DigiTool(name, **kwargs))
    return acc


def MDT_Response_DigiToolCfg(flags, name="MDT_Response_DigiTool",**kwargs):
    """Return a configured MDT_Response_DigiTool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("DoQballGamma", (flags.Input.SpecialConfiguration.get("MDT_QballConfig", "False") == "True"))
    MDT_Response_DigiTool = CompFactory.MDT_Response_DigiTool
    acc.setPrivateTools(MDT_Response_DigiTool(name, **kwargs))
    return acc


def MDT_DigitizationToolCommonCfg(flags, name="MdtDigitizationTool", **kwargs):
    """Return ComponentAccumulator with common MdtDigitizationTool config"""
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg # MT-safe conditions access
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg

    acc = ComponentAccumulator()
    acc.merge(MdtCondDbAlgCfg(flags))
    acc.merge(MdtCalibDbAlgCfg(flags))

    ### configuration arguments not yet migrated to the PhaseII geometry
    if not flags.Muon.usePhaseIIGeoSetup:
        kwargs.setdefault("DiscardEarlyHits", True)
        kwargs.setdefault("UseTof", flags.Beam.Type is not BeamType.Cosmics)
        kwargs.setdefault("DoQballCharge", (flags.Input.SpecialConfiguration.get("MDT_QballConfig", "False") == "True"))
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(MDT_Response_DigiToolCfg(flags)))
    else:
        ### Use the simple digitization tool as a first start
        from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
        acc.merge(ActsGeometryContextAlgCfg(flags))
        kwargs.setdefault("useTwinTubes", True)
        if kwargs["useTwinTubes"]:
            from MuonConfig.MuonCablingConfig import MdtTwinTubeMapCondAlgCfg
            acc.merge(MdtTwinTubeMapCondAlgCfg(flags))
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(RT_Relation_DB_DigiToolCfg(flags)))
        kwargs.setdefault("SimHitKey", "xMdtSimHits")
        kwargs.setdefault("StreamName", "MdtDigitForklifting")

    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", MDT_FirstXing())
        kwargs.setdefault("LastXing", MDT_LastXing())
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    if not flags.Muon.usePhaseIIGeoSetup:
         acc.setPrivateTools(CompFactory.MdtDigitizationTool(name, **kwargs))
    else:
         acc.setPrivateTools(CompFactory.MuonR4.MdtDigitizationTool(name, **kwargs))
    return acc


def MDT_DigitizationToolCfg(flags, name="MdtDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured MdtDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(MDT_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("OutputObjectName", "MDT_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "MDT_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "MDT_SDO")
    tool = acc.popToolsAndMerge(MDT_DigitizationToolCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def MDT_OverlayDigitizationToolCfg(flags, name="Mdt_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with MdtDigitizationTool configured for Overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "MDT_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "MDT_SDO")
    kwargs.setdefault("PileUpMergeSvc", '')
    return MDT_DigitizationToolCommonCfg(flags, name, **kwargs)


def MDT_OutputCfg(flags):
    """Return ComponentAccumulator with Output for MDT. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["MdtCsmContainer#*"]
        if flags.Digitization.EnableTruth:
            for pref in  [flags.Overlay.SigPrefix, flags.Overlay.BkgPrefix, ""]:
                ## Legacy SDO container
                ItemList += [f"MuonSimDataCollection#{pref}MDT_SDO"]
                ## New SDO container
                ItemList += [f"xAOD::MuonSimHitContainer#{pref}MDT_SDO", f"xAOD::MuonSimHitAuxContainer#{pref}MDT_SDOAux."]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def MDT_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for MDT digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(MDT_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def MDT_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with MDT Overlay digitization"""
    acc = MuonGeoModelCfg(flags)
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    acc.merge(MdtCalibDbAlgCfg(flags))

    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        if flags.Muon.usePhaseIIGeoSetup:
            acc.merge(SGInputLoaderCfg(flags, ["xAOD::MuonSimHitContainer#xMdtSimHits",
                                               "xAOD::MuonSimHitAuxContainer#xMdtSimHitsAux."]))
        else:            
            acc.merge(SGInputLoaderCfg(flags, ["MDTSimHitCollection#MDT_Hits"]))

    kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(MDT_OverlayDigitizationToolCfg(flags)))
    
    if flags.Concurrency.NumThreads > 0:
       kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.MuonDigitizer(name="MDT_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def MDT_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for MDT digitization and Output"""
    acc = MDT_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(MDT_OutputCfg(flags))
    return acc


def MDT_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with MDT digitization and Digit to MDTCSM RDO"""
    acc = MDT_DigitizationCfg(flags)
    acc.merge(MDTCablingConfigCfg(flags))
    acc.merge(MdtDigitToMdtRDOCfg(flags))
    return acc
