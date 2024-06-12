# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSystematicsTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTrackTruthOriginToolCfg(flags, name="InDetTrackTruthOriginTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("isFullPileUpTruth", flags.Digitization.PileUp \
                      and flags.Digitization.DigiSteeringConf in ['StandardPileUpToolsAlg', \
                                                                  'StandardInTimeOnlyTruthPileUpToolsAlg', \
                                                                  'StandardInTimeOnlyGeantinoTruthPileUpToolsAlg'])

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackTruthOriginTool(name, **kwargs))
    return acc

def InDetTrackTruthFilterToolCfg(flags, name="InDetTrackTruthFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackTruthFilterTool(name, **kwargs))
    return acc

def JetTrackFilterToolCfg(flags, name="JetTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    from AthenaConfiguration.Enums import LHCPeriod
    # Run 3 recommendations (MC23): https://indico.cern.ch/event/1424738/#20-run-3-recommendations-fake
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        kwargs.setdefault("FakeUncertainty", 0.30)
    # Run 2 recommendations (MC20): https://cds.cern.ch/record/2859907
    else:
        kwargs.setdefault("FakeUncertainty", 0.35)

    acc.setPrivateTools(CompFactory.InDet.JetTrackFilterTool(name, **kwargs))
    return acc

def InclusiveTrackFilterToolCfg(flags, name="InclusiveTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.InDet.InclusiveTrackFilterTool(name, **kwargs))
    return acc

def TrackSystematicsAlgCfg(flags, name="InDetTrackSystematicsAlg", **kwargs):
    acc = ComponentAccumulator()

    if "TrackFilterToolLRT" not in kwargs:
        kwargs.setdefault("TrackFilterToolLRT", acc.popToolsAndMerge(
            InclusiveTrackFilterToolCfg(flags)))

    if "TrackFilterToolSTD" not in kwargs:
        kwargs.setdefault("TrackFilterToolSTD", acc.popToolsAndMerge(
            InDetTrackTruthFilterToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TrackSystematicsAlg(name, **kwargs))
    return acc
