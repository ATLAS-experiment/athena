# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TrajectoryBuilderCfg(flags, name = "MuonDecayTruthTrajectoryBuilder", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.Muon.MuonDecayTruthTrajectoryBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
def MuonTrackTruthToolCfg(flags, name = "MuonTrackTruthTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("TruthTrajectoryBuilder", result.popToolsAndMerge(TrajectoryBuilderCfg(flags)))    
    the_tool = CompFactory.Muon.MuonTrackTruthTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonTrackPerformanceAlgCfg(flags, name = "MuonTrackPerformanceAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg, MuonTrackSummaryHelperToolCfg
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("TrackTruthTool", result.popToolsAndMerge(MuonTrackTruthToolCfg(flags)))
    kwargs.setdefault("SummaryHelperTool", result.popToolsAndMerge(MuonTrackSummaryHelperToolCfg(flags)))
    kwargs.setdefault("DoTruth", flags.Input.isMC)
    the_alg = CompFactory. MuonTrackPerformanceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonPerformanceAlgCfg(flags, name = "MuonPerformanceAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonPerformanceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonSegmentPerformanceAlgCfg(flags, name = "MuonSegmentPerformanceAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonSegmentPerformanceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonTrackStatisticsToolCfg(flags, name = "MuonTrackStatisticsTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.MuonTrackStatisticsTool(name, **kwargs))
    return result
def MuonTrackStatisticsAlgCfg(flags, name = "MuonTrackStatisticsAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("StatTool", result.popToolsAndMerge(MuonTrackStatisticsToolCfg(flags)))
    return result
