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
# Expert-only track-time accessor tools. These were previously provided by the
# standalone HGTDTrackTimeInterface package and are now part of HGTD_Analysis.
#
# All of them are for HGTD expert studies and internal validation only, and
# must not be scheduled in standard reconstruction jobs -- reconstruction and
# analysis code takes the track time from the xAOD::TrackParticle EDM.
#

def HGTD_ExpertTrackTimeFromClustersToolCfg(flags, name='ExpertTrackTimeFromClusters',
                                            **kwargs):
    """Re-derives the track time from the per-layer HGTD cluster decorations."""
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", True)
    kwargs.setdefault("UseTimeConsistency", True)
    kwargs.setdefault("DeltaTCut", 2.0)
    kwargs.setdefault("TimeChi2Cut", 1.5)
    acc.setPrivateTools(CompFactory.HGTD.ExpertTrackTimeFromClustersTool(name, **kwargs))

    return acc


def HGTD_ExpertTrackTimeFromClustersNoSelectionToolCfg(
        flags, name='ExpertTrackTimeFromClustersNoSelection', **kwargs):
    """As above, but with the last-hit, time-consistency and minimum-hit cuts off."""
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", False)
    kwargs.setdefault("UseTimeConsistency", False)
    kwargs.setdefault("DeltaTCut", 2.0)
    kwargs.setdefault("TimeChi2Cut", 1.5)
    kwargs.setdefault("UseMinNHits", False)
    acc.setPrivateTools(CompFactory.HGTD.ExpertTrackTimeFromClustersTool(name, **kwargs))

    return acc


def HGTD_ExpertTrackTimeFromEDMToolCfg(flags, name='ExpertTrackTimeFromEDM', **kwargs):
    """Hands back xAOD::TrackParticle::time() unchanged, as a reference point."""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.HGTD.ExpertTrackTimeFromEDMTool(name, **kwargs))

    return acc


def HGTD_ExpertTrackTimeFromSummaryToolCfg(flags, name='ExpertTrackTimeFromSummary',
                                           **kwargs):
    """Re-applies the HGTD selection by decoding the persisted HGTD_summaryinfo."""
    acc = ComponentAccumulator()
    kwargs.setdefault("UseLastHitCut", True)
    kwargs.setdefault("UseMinNHits", True)
    acc.setPrivateTools(CompFactory.HGTD.ExpertTrackTimeFromSummaryTool(name, **kwargs))

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

    time_from_clusters = acc.popToolsAndMerge(
        HGTD_ExpertTrackTimeFromClustersToolCfg(flags))
    time_from_clusters_no_sel = acc.popToolsAndMerge(
        HGTD_ExpertTrackTimeFromClustersNoSelectionToolCfg(flags))
    time_from_summary = acc.popToolsAndMerge(
        HGTD_ExpertTrackTimeFromSummaryToolCfg(flags))

    all_tracks_sel_tool = acc.popToolsAndMerge(
        HGTD_AllTracksSelectionToolCfg(flags))
    no_early_decay_tracks_sel_tool = acc.popToolsAndMerge(
        HGTD_NoEarlyDecayTracksSelectionToolCfg(flags))

    kwargs.setdefault("DirectoryName", "/HGTD_ANA/")
    kwargs.setdefault("TrackTimeTools",
                      [time_from_clusters, time_from_clusters_no_sel, time_from_summary])
    kwargs.setdefault("TrackSelectionTools",
                      [all_tracks_sel_tool, no_early_decay_tracks_sel_tool])

    # Note! change the below if you have a use case where you put a specific
    # selection into SG and schedule the performance algo afterwards!
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")

    acc.addEventAlgo(CompFactory.HGTD_TrkTimePerformanceStudies(name, **kwargs))

    acc.merge(OutputCfg(flags, output_name=output_name))

    return acc


def main():
    """Run HGTD_TrkTimePerformanceStudies over an AOD.

    This is the single entry point shared with share/JO_Performance_Studies.py.

    The input AOD must have been produced with
      --preExec 'all:flags.Tracking.writeExtendedHGTDInfo=True'
    otherwise the extended HGTD track decorations the track-time accessor
    tools read (HGTD_cluster_time, HGTD_primary_expected, HGTD_summaryinfo,
    ...) are stripped from the AOD and all histograms come out empty.
    """
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = -1
    flags.fillFromArgs()  # set the input with: --filesInput=...

    if not flags.Input.Files:
        raise SystemExit("ERROR: no input file given, use --filesInput=<AOD.pool.root>")

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(HGTD_TrkTimePerformanceStudiesCfg(flags))

    return cfg.run().isFailure()


if __name__ == "__main__":
    import sys
    sys.exit(main())
