# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PixelClusterRetrieverCfg(flags, name="PixelClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PixelTruthMap", "")
    the_tool = CompFactory.JiveXML.PixelClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiClusterRetrieverCfg(flags, name="SiClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("SCT_TruthMap", "")
    the_tool = CompFactory.JiveXML.SiClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiSpacePointRetrieverCfg(flags, name="SiSpacePointRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PRD_TruthPixel", "")
        kwargs.setdefault("PRD_TruthSCT", "")
    the_tool = CompFactory.JiveXML.SiSpacePointRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TRTRetrieverCfg(flags, name="TRTRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("TRTTruthMap", "")
    the_tool = CompFactory.JiveXML.TRTRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TrackRetrieverCfg(flags, name="TrackRetriever", **kwargs):
    # Based on TrkJiveXML_DataTypes
    result = ComponentAccumulator()
    # FIXME - this is copied from TrkJiveXML_DataTypes.py, but we can do better
    kwargs.setdefault("PriorityTrackCollection", "Tracks")
    kwargs.setdefault(
        "OtherTrackCollections",
        [
            "CombinedMuonTracks",
            "MuonSpectrometerTracks",
            "ConvertedStacoTracks",
            "ConvertedMuIdCBTracks",
            "CombinedInDetTracks",
            "GSFTracks",
        ],
    )
    ### The Event Filter track collections are not written to XML by default.
    ### To write them out, you must uncomment the following line:
    # kwargs.setdefault("DoWriteHLT", True)
    ### switch residual data off:
    kwargs.setdefault("DoWriteResiduals", False)
    the_tool = CompFactory.JiveXML.TrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def BeamSpotRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.BeamSpotRetriever(name="BeamSpotRetriever", **kwargs)
    result.addPublicTool(the_tool)
    return result


def InDetRetrieversCfg(flags):
    result = ComponentAccumulator()
    # Do we need to add equivalent of InDetFlags.doSlimming=False (in JiveXML_RecEx_config.py)? If so, why?
    # Following is based on InDetJiveXML_DataTypes.py and TrkJiveXML_DataTypes.py
    if flags.Detector.EnablePixel and flags.Detector.GeometryPixel:
        result.merge(PixelClusterRetrieverCfg(flags))

    if flags.Detector.EnableID and flags.Detector.GeometryID and flags.Detector.EnablePixel and flags.Detector.GeometryPixel and flags.Detector.EnableSCT and flags.Detector.GeometrySCT:
            result.merge(SiClusterRetrieverCfg(flags))
            result.merge(SiSpacePointRetrieverCfg(flags))
            result.merge(TrackRetrieverCfg(flags))

    if flags.Detector.EnableTRT and flags.Detector.GeometryTRT:
        result.merge(TRTRetrieverCfg(flags))

    if not flags.OnlineEventDisplays.OfflineTest:
        result.merge(BeamSpotRetrieverCfg(flags))

    return result
