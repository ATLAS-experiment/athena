# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def OutputCfg(flags, output_name='HGTD_PerformanceStudies_output'):
    acc = ComponentAccumulator()

    svc = CompFactory.THistSvc(name="THistSvc")
    svc.Output = [f"HGTD_ANA DATAFILE='{output_name}.root' OPT='RECREATE'"]
    acc.addService(svc)

    return acc


#
# Track-time accessor tools. These were previously provided by the standalone
# HGTDTrackTimeInterface package and are now part of HGTD_Analysis.
#

def HGTD_TrackTimeAccToolCfg(flags, name='TrackTimeAccTool', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", True)
    kwargs.setdefault("UseTimeConsistency", True)
    kwargs.setdefault("DeltaTCut", 2.0)
    kwargs.setdefault("TimeChi2Cut", 1.5)
    acc.setPrivateTools(CompFactory.HGTD.TrackTimeAccTool(name, **kwargs))

    return acc


def HGTD_TrackTimeAccToolDefaultCfg(flags, name='TrackTimeAccToolDefault', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", False)
    kwargs.setdefault("UseTimeConsistency", False)
    kwargs.setdefault("DeltaTCut", 2.0)
    kwargs.setdefault("TimeChi2Cut", 1.5)
    kwargs.setdefault("UseMinNHits", False)
    acc.setPrivateTools(CompFactory.HGTD.TrackTimeAccTool(name, **kwargs))

    return acc


def HGTD_TrackTimeAccTool2Cfg(flags, name='TrackTimeAccTool2', **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.HGTD.TrackTimeAccTool2(name, **kwargs))

    return acc


def HGTD_TrackTimeAccTool3Cfg(flags, name='TrackTimeAccTool3', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", True)
    kwargs.setdefault("UseMinNHits", True)
    acc.setPrivateTools(CompFactory.HGTD.TrackTimeAccTool3(name, **kwargs))

    return acc


#
# Track selection tools.
#

def HGTD_NoEarlyDecayTracksSelectionToolCfg(flags, name='NoEarlyDecayTracksSelection', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("MinRadiusProduction", 0.)
    kwargs.setdefault("MazRadiusProduction", 10.)
    kwargs.setdefault("MinZProduction", 0.)
    kwargs.setdefault("MaxZProduction", 100.)
    kwargs.setdefault("MinRadiusDecay", 0.)
    kwargs.setdefault("MazRadiusDecay", 99999999999.)
    kwargs.setdefault("MinZDecay", 3479.)  # just after last HGTD layer
    kwargs.setdefault("MaxZDecay", 99999999999.)
    acc.setPrivateTools(CompFactory.HGTD_TrackDecaySelectionTool(name, **kwargs))

    return acc


def HGTD_AllTracksSelectionToolCfg(flags, name='AllTracksSelection', **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.HGTD_AllTracksSelectionTool(name, **kwargs))

    return acc


#
# The performance-study algorithm.
#

def HGTD_TrkTimePerformanceStudiesCfg(flags, name='HGTD_TrkTimePerformanceStudies',
                                      output_name='HGTD_PerformanceStudies_output', **kwargs):
    acc = ComponentAccumulator()

    timetool = acc.popToolsAndMerge(HGTD_TrackTimeAccToolCfg(flags, 'TrackTimeAccTool'))
    timetool2 = acc.popToolsAndMerge(HGTD_TrackTimeAccToolDefaultCfg(flags, 'TrackTimeAccToolDefault'))
    timetool3 = acc.popToolsAndMerge(HGTD_TrackTimeAccTool3Cfg(flags, 'TrackTimeAccTool3'))

    all_tracks_sel_tool = acc.popToolsAndMerge(
        HGTD_AllTracksSelectionToolCfg(flags, 'AllTracksSelection'))
    no_early_decay_tracks_sel_tool = acc.popToolsAndMerge(
        HGTD_NoEarlyDecayTracksSelectionToolCfg(flags, 'NoEarlyDecayTracksSelection'))

    kwargs.setdefault("DirectoryName", "/HGTD_ANA/")
    kwargs.setdefault("TrackTimeTools", [timetool, timetool2, timetool3])
    kwargs.setdefault("TrackSelectionTools",
                      [all_tracks_sel_tool, no_early_decay_tracks_sel_tool])

    # Note! change the below if you have a use case where you put a specific
    # selection into SG and schedule the performance algo afterwards!
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")

    acc.addEventAlgo(CompFactory.HGTD_TrkTimePerformanceStudies(name, **kwargs))

    acc.merge(OutputCfg(flags, output_name=output_name))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = -1
    flags.fillFromArgs()  # set the input with: --filesInput=...
    flags.lock()

    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(HGTD_TrkTimePerformanceStudiesCfg(flags))
    import sys
    sys.exit(cfg.run().isFailure())
