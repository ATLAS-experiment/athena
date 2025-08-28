# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


def PixelClusterRetrieverCfg(flags, name="PixelClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PixelTruthMap", "")
    the_tool = CompFactory.JiveXML.PixelClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def SiClusterRetrieverCfg(flags, name="SiClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("SCT_TruthMap", "")
    the_tool = CompFactory.JiveXML.SiClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def SiSpacePointRetrieverCfg(flags, name="SiSpacePointRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PRD_TruthPixel", "")
        kwargs.setdefault("PRD_TruthSCT", "")
    the_tool = CompFactory.JiveXML.SiSpacePointRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def TRTRetrieverCfg(flags, name="TRTRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("TRTTruthMap", "")
    the_tool = CompFactory.JiveXML.TRTRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def TrackRetrieverCfg(flags, name="TrackRetriever", **kwargs):
    # Based on TrkJiveXML_DataTypes
    result = ComponentAccumulator()

    from MuonConfig.MuonRecToolsConfig import MuonTrackSummaryHelperToolCfg
    muonSummaryHelperTool=result.popToolsAndMerge(MuonTrackSummaryHelperToolCfg(flags))

    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
    InDetTrackSummaryTool=result.popToolsAndMerge(InDetTrackSummaryToolCfg(flags,MuonSummaryHelperTool=muonSummaryHelperTool))
    kwargs.setdefault("TrackSummaryTool", InDetTrackSummaryTool)

    from TrkConfig.TrkResidualPullCalculatorConfig import (ResidualPullCalculatorCfg)
    ResidualPullCalculator=result.addPublicTool(result.popToolsAndMerge(ResidualPullCalculatorCfg(flags)))
    kwargs.setdefault("ResidualPullCalculator", ResidualPullCalculator)
    if not flags.Input.isMC:
        kwargs.setdefault("TruthCollections",[""])
    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("DoWriteResiduals", False)
    the_tool = CompFactory.JiveXML.TrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def VertexRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.VertexRetriever(name="VertexRetriever", **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def SegmentRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.SegmentRetriever(name="SegmentRetriever", **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def BeamSpotRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.BeamSpotRetriever(name="BeamSpotRetriever", **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def PixelRDORetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.PixelRDORetriever(name="PixelRDORetriever", **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def SCTRDORetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.SCTRDORetriever(name="SCTRDORetriever", **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def InDetRetrieversCfg(flags):
    result = ComponentAccumulator()
    tools = []
    # Do we need to add equivalent of InDetFlags.doSlimming=False (in JiveXML_RecEx_config.py)? If so, why?
    # Following is based on InDetJiveXML_DataTypes.py and TrkJiveXML_DataTypes.py

    haveRDO = (
        flags.Input.Format is Format.BS or "StreamRDO" in flags.Input.ProcessingTags
    )

    if flags.Detector.EnablePixel and flags.Detector.GeometryPixel:
        tools += [result.getPrimaryAndMerge(PixelClusterRetrieverCfg(flags))]
        if haveRDO:
            tools += [result.getPrimaryAndMerge(PixelRDORetrieverCfg(flags))]

    if flags.Detector.EnableID and flags.Detector.GeometryID and flags.Detector.EnablePixel and flags.Detector.GeometryPixel and flags.Detector.EnableSCT and flags.Detector.GeometrySCT:
            tools += [result.getPrimaryAndMerge(SiClusterRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(SiSpacePointRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(SegmentRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(VertexRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(TrackRetrieverCfg(flags))]

    if flags.Detector.EnableTRT and flags.Detector.GeometryTRT:
        tools += [result.getPrimaryAndMerge(TRTRetrieverCfg(flags))]

    if haveRDO and flags.Detector.EnableSCT and flags.Detector.GeometrySCT:
        tools += [result.getPrimaryAndMerge(SCTRDORetrieverCfg(flags))]

    if not flags.OnlineEventDisplays.OfflineTest:
        tools += [result.getPrimaryAndMerge(BeamSpotRetrieverCfg(flags))]

    return result, tools
