# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# ------------------------------------------------------------
#
# ----------- Setup Si Pattern for New tracking
#
# ------------------------------------------------------------


def ITkTrackingSiPatternCfg(flags,
                            InputCollections=None,
                            ResolvedTrackCollectionKey=None,
                            SiSPSeededTrackCollectionKey=None,
                            ClusterSplitProbContainer=''):
    acc = ComponentAccumulator()
    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            ITkTrackPRD_AssociationCfg)
        acc.merge(ITkTrackPRD_AssociationCfg(
            flags,
            name=('ITkTrackPRD_Association' +
                  flags.Tracking.ActiveConfig.extension),
            TracksName=list(InputCollections)))

    runTruth = True
    runActsTrackTruth = False

    # Can use FastTrackFinder instead of SiSPSeededTrackFinder
    if flags.Tracking.useITkFTF:

        # ------------------------------------------------------------
        #
        # ----------- FastTrackFinder
        #
        # ------------------------------------------------------------

        from TrigFastTrackFinder.ITkFastTrackFinderStandaloneConfig import (
            ITkFastTrackFinderStandaloneCfg)
        acc.merge(ITkFastTrackFinderStandaloneCfg(
            flags, SiSPSeededTrackCollectionKey))

    else:

        # ------------------------------------------------------------
        #
        # ----------- SiSPSeededTrackFinder
        #
        # ------------------------------------------------------------

        runTruth = flags.Tracking.ActiveConfig.doAthenaTrack or flags.Tracking.ActiveConfig.doActsToAthenaTrack or flags.Tracking.ActiveConfig.doGNNTrack


        # Athena Track
        if flags.Tracking.ActiveConfig.doAthenaTrack:

            from InDetConfig.SiSPSeededTrackFinderConfig import (
                ITkSiSPSeededTrackFinderCfg)
            SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderCfg
            if flags.Tracking.ActiveConfig.extension == "Conversion":
                from InDetConfig.SiSPSeededTrackFinderConfig import (
                    ITkSiSPSeededTrackFinderROIConvCfg)
                SiSPSeededTrackFinderCfg = ITkSiSPSeededTrackFinderROIConvCfg

            acc.merge(SiSPSeededTrackFinderCfg(
                flags,
                TracksLocation=SiSPSeededTrackCollectionKey))

        # GNN Track
        if flags.Tracking.ActiveConfig.doGNNTrack:
            from InDetGNNTracking.InDetGNNTrackingConfig import GNNTrackMakerCfg
            acc.merge(GNNTrackMakerCfg(
                flags,
                TracksLocation=SiSPSeededTrackCollectionKey))

        # ACTS seed
        if flags.Tracking.ActiveConfig.doActsSeed:

            from ActsConfig.ActsSeedingConfig import (
                ActsSeedingCfg)
            acc.merge(ActsSeedingCfg(flags))

        # ACTS track
        if flags.Tracking.ActiveConfig.doActsTrack:
            from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
            acc.merge(ActsTrackFindingCfg(flags))
            runActsTrackTruth = flags.Tracking.doTruth

        # Convert Tracks Acts -> Athena (before ambi)
        if flags.Tracking.ActiveConfig.doActsToAthenaTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               TracksLocation=SiSPSeededTrackCollectionKey))

        # Convert tracks Athena -> Acts (before ambi)
        if flags.Tracking.ActiveConfig.doAthenaToActsTrack:
            from ActsConfig.ActsEventCnvConfig import TrkToActsConvertorAlgCfg
            acc.merge(TrkToActsConvertorAlgCfg(flags,
                                               TrackContainerLocation="ActsTracks",
                                               TrackCollectionKeys=[SiSPSeededTrackCollectionKey]))

        if flags.Tracking.ActiveConfig.doActsTrack and not flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            # create track particles from acts tracks
            from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
            # The following few lines will disappear once we have imposed a proper nomenclature for our algorithms and collection
            prefix = "Acts"
            if 'Validate' not in flags.Tracking.ActiveConfig.extension and 'BenchmarkSpot' not in flags.Tracking.ActiveConfig.extension:
                prefix = flags.Tracking.ActiveConfig.extension
            acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags,
                                                        f"{prefix}TrackToAltTrackParticleCnvAlg",
                                                        ACTSTracksLocation=f'{prefix}Tracks',
                                                        TrackParticlesOutKey=f'{prefix}TrackParticlesAlt'))
            
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    if flags.Tracking.doTruth and runTruth:
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=SiSPSeededTrackCollectionKey,
            DetailedTruth=SiSPSeededTrackCollectionKey+"DetailedTruth",
            TracksTruth=SiSPSeededTrackCollectionKey+"TruthCollection"))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------
    runTruth = True

    if flags.Tracking.doITkFastTracking and flags.Tracking.ActiveConfig.doAthenaTrack:

        from TrkConfig.TrkCollectionAliasAlgConfig import CopyAlgForAmbiCfg
        acc.merge(CopyAlgForAmbiCfg(
            flags,
            "ITkCopyAlgForAmbi"+flags.Tracking.ActiveConfig.extension,
            CollectionName=SiSPSeededTrackCollectionKey,  # Input
            AliasName=ResolvedTrackCollectionKey))       # Output

    else:
        # If we run Athena tracking we also want CTIDE ambi
        if flags.Tracking.ActiveConfig.doAthenaAmbiguityResolution:
            # with Acts.doAmbiguityResolution the converter will directly produce
            # tracks with the key ResolvedTrackCollectionKey
            from TrkConfig.TrkAmbiguitySolverConfig import (
                ITkTrkAmbiguityScoreCfg, ITkTrkAmbiguitySolverCfg)
            acc.merge(ITkTrkAmbiguityScoreCfg(
                flags,
                SiSPSeededTrackCollectionKey=SiSPSeededTrackCollectionKey,
                ClusterSplitProbContainer=ClusterSplitProbContainer))
            
            acc.merge(ITkTrkAmbiguitySolverCfg(
                flags,
                ResolvedTrackCollectionKey=ResolvedTrackCollectionKey))
            
            runTruth = True

        # If we run Acts tracking we may want Acts ambi, depending on the flag
        if flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            # Schedule ACTS ambi. resolution and eventually the track convertions  
            from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
            acc.merge(ActsAmbiguityResolutionCfg(flags))
            runTruth = False

        if flags.Tracking.ActiveConfig.doActsToAthenaResolvedTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(flags,
                                               TracksLocation=ResolvedTrackCollectionKey))
            runTruth = False

        if flags.Tracking.ActiveConfig.doActsTrack and flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            # create track particles from resolved acts tracks
            from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
            # The following few lines will disappear once we have imposed a proper nomenclature for our algorithms and collection
            prefix = "Acts"
            if 'Validate' not in flags.Tracking.ActiveConfig.extension and 'BenchmarkSpot' not in flags.Tracking.ActiveConfig.extension:
                prefix = flags.Tracking.ActiveConfig.extension
            acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, f"{prefix}ResolvedTrackToAltTrackParticleCnvAlg",
                                                        ACTSTracksLocation=f'{prefix}ResolvedTracks',
                                                        TrackParticlesOutKey=f'{prefix}ResolvedTrackParticlesAlt'))


    if flags.Tracking.doTruth and runTruth:
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=ResolvedTrackCollectionKey,
            DetailedTruth=ResolvedTrackCollectionKey+"DetailedTruth",
            TracksTruth=ResolvedTrackCollectionKey+"TruthCollection"))

    if runActsTrackTruth :
        from ActsConfig.ActsTruthConfig import TrackToTruthAssociationCfg
        acts_tracks='ActsTracks' if not flags.Acts.doAmbiguityResolution else 'ActsResolvedTracks'
        acc.merge(TrackToTruthAssociationCfg(flags,
                                             ACTSTracksLocation=acts_tracks,
                                             AssociationMapOut=acts_tracks+"ToTruthParticleAssociation"))


    return acc
