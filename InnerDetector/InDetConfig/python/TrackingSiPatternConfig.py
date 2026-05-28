# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------
def TrackingSiPatternCfg(flags,
                         InputCollections = None,
                         ResolvedTrackCollectionKey = None,
                         SiSPSeededTrackCollectionKey = None ,
                         ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            InDetTrackPRD_AssociationCfg)
        acc.merge(InDetTrackPRD_AssociationCfg(
            flags,
            name = ('InDetTrackPRD_Association' +
                    flags.Tracking.ActiveConfig.extension),
            TracksName = list(InputCollections)))

    # ------------------------------------------------------------
    #
    # ----------- SiSPSeededTrackFinder
    #
    # ------------------------------------------------------------

    from InDetConfig.SiSPSeededTrackFinderConfig import (
        SiSPSeededTrackFinderCfg, SiSPSeededTrackFinderRoICfg)
    if flags.Tracking.ActiveConfig.extension == "LowPtRoI":
        # Dedicated track finding algorithm supporting z-window RoI
        acc.merge(SiSPSeededTrackFinderRoICfg(
            flags,
            TracksLocation = SiSPSeededTrackCollectionKey))        
    else:
        acc.merge(SiSPSeededTrackFinderCfg(
            flags,
            TracksLocation = SiSPSeededTrackCollectionKey))

    from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
    if flags.Tracking.doTruth:
        acc.merge(InDetTrackTruthCfg(
            flags,
            Tracks = SiSPSeededTrackCollectionKey,
            DetailedTruth = SiSPSeededTrackCollectionKey+"DetailedTruth",
            TracksTruth = SiSPSeededTrackCollectionKey+"TruthCollection"))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    from TrkConfig.TrkAmbiguitySolverConfig import (
        TrkAmbiguityScoreCfg, TrkAmbiguitySolverCfg)
    acc.merge(TrkAmbiguityScoreCfg(
        flags,
        SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
        ClusterSplitProbContainer = ClusterSplitProbContainer))

    acc.merge(TrkAmbiguitySolverCfg(
        flags,
        ResolvedTrackCollectionKey = ResolvedTrackCollectionKey,
        ClusterSplitProbContainer = ClusterSplitProbContainer))

    if flags.Tracking.doTruth:
        acc.merge(InDetTrackTruthCfg(
            flags,
            Tracks = ResolvedTrackCollectionKey,
            DetailedTruth = ResolvedTrackCollectionKey+"DetailedTruth",
            TracksTruth = ResolvedTrackCollectionKey+"TruthCollection"))

    return acc


