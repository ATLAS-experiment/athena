# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetRecStatistics package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import BeamType

def InDetRecStatisticsAlgCfg(flags, name='InDetRecStatistics', **kwargs):
    acc = ComponentAccumulator()

    if "TruthToTrackTool" not in kwargs and flags.Tracking.doTruth:
        from TrkConfig.TrkTruthCreatorToolsConfig import TruthToTrackToolCfg
        TruthToTrackTool = acc.popToolsAndMerge(TruthToTrackToolCfg(flags))
        acc.addPublicTool(TruthToTrackTool)
        kwargs.setdefault("TruthToTrackTool", TruthToTrackTool)

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        SummaryTool = acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags))
        acc.addPublicTool(SummaryTool)
        kwargs.setdefault("SummaryTool", SummaryTool)

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", acc.addPublicTool(
            acc.popToolsAndMerge(ResidualPullCalculatorCfg(flags))))
        
    kwargs.setdefault("PrintSecondary", True)
    kwargs.setdefault("UseTrackSummary", True)
    kwargs.setdefault("DoTruth", flags.Tracking.doTruth)
    kwargs.setdefault("fakeTrackCut", 0.8)
    kwargs.setdefault("fakeTrackCut2", 0.5)

    if flags.Beam.Type in [BeamType.Cosmics, BeamType.SingleBeam]:
        kwargs.setdefault("minPt", 0.*Units.GeV)
        kwargs.setdefault("maxEta", 9999.)
        kwargs.setdefault("maxRStartPrimary",   9999999.)
        kwargs.setdefault("maxRStartSecondary", 9999999.)
        kwargs.setdefault("maxZStartPrimary",   9999999.)
        kwargs.setdefault("maxZStartSecondary", 9999999.)
        kwargs.setdefault("minREndPrimary",     0.)
        kwargs.setdefault("minREndSecondary",   0.)
        kwargs.setdefault("minZEndPrimary",     0.)
        kwargs.setdefault("minZEndSecondary",   0.)

    else:
        kwargs.setdefault("minPt", 1.*Units.GeV)
        kwargs.setdefault("maxEta", 2.7)
        kwargs.setdefault("maxRStartPrimary",   25.)
        kwargs.setdefault("maxRStartSecondary", 560.)
        kwargs.setdefault("maxZStartPrimary",   320.)
        kwargs.setdefault("maxZStartSecondary", 1500.)
        kwargs.setdefault("minREndPrimary",     400.)
        kwargs.setdefault("minREndSecondary",   1000.)
        kwargs.setdefault("minZEndPrimary",     2300.)
        kwargs.setdefault("minZEndSecondary",   2700.)
    
    acc.addEventAlgo(CompFactory.InDet.InDetRecStatisticsAlg(name, **kwargs))
    return acc

def ITkRecStatisticsAlgCfg(flags, name='ITkRecStatistics', **kwargs):
    acc = ComponentAccumulator()

    if "TruthToTrackTool" not in kwargs and flags.Tracking.doTruth:
        from TrkConfig.TrkTruthCreatorToolsConfig import TruthToTrackToolCfg
        TruthToTrackTool = acc.popToolsAndMerge(TruthToTrackToolCfg(flags))
        acc.addPublicTool(TruthToTrackTool)
        kwargs.setdefault("TruthToTrackTool", TruthToTrackTool)

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
        SummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(flags))
        acc.addPublicTool(SummaryTool)
        kwargs.setdefault("SummaryTool", SummaryTool)

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", acc.addPublicTool(
            acc.popToolsAndMerge(ResidualPullCalculatorCfg(flags))))
        
    kwargs.setdefault("PrintSecondary", True)
    kwargs.setdefault("UseTrackSummary", True)
    kwargs.setdefault("DoTruth", flags.Tracking.doTruth)
    kwargs.setdefault("fakeTrackCut", 0.8)
    kwargs.setdefault("fakeTrackCut2", 0.5)

    kwargs.setdefault("minPt", 1.*Units.GeV)
    kwargs.setdefault("maxEta", 4.0)
    kwargs.setdefault("maxRStartPrimary",   25.)
    kwargs.setdefault("maxRStartSecondary", 560.)
    kwargs.setdefault("maxZStartPrimary",   320.)
    kwargs.setdefault("maxZStartSecondary", 1500.)
    kwargs.setdefault("minREndPrimary",     400.)
    kwargs.setdefault("minREndSecondary",   1000.)
    kwargs.setdefault("minZEndPrimary",     2300.)
    kwargs.setdefault("minZEndSecondary",   2700.)

    acc.addEventAlgo(CompFactory.InDet.InDetRecStatisticsAlg(name, **kwargs))
    return acc
