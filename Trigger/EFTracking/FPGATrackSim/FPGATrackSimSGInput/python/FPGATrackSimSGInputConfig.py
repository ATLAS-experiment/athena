# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.SystemOfUnits import GeV

def FPGATrackSimSGInputToolCfg(flags):
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolatorTool = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))

    from TrkConfig.TrkTruthCreatorToolsConfig import TruthToTrackToolCfg
    truthToTrackTool = acc.popToolsAndMerge(TruthToTrackToolCfg(flags))

    FPGATrackSimSGInputTool = CompFactory.FPGATrackSimSGToRawHitsTool(maxEta=3.2, minPt=0.8 * GeV,
        Extrapolator = extrapolatorTool, TruthToTrackTool = truthToTrackTool)
    acc.setPrivateTools(FPGATrackSimSGInputTool)

    return acc

def FPGATrackSimSGInputCfg(flags,**kwargs):
    """
    Configure FPGATrackSim wrappers generation, outFile will be taken from flags in the future
    """

    acc = ComponentAccumulator()
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    wrapperAlg = CompFactory.TrigFPGATrackSimRawHitsWrapperAlg(
        InputTool=acc.popToolsAndMerge(FPGATrackSimSGInputToolCfg(flags)),
        OutFileName=flags.Trigger.FPGATrackSim.wrapperFileName,
        WrapperMetaData=flags.Trigger.FPGATrackSim.wrapperMetaData
    )
    acc.addEventAlgo(wrapperAlg)

    return acc
# how to run? See README file
