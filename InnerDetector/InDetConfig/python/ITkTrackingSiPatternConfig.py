# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
                            ClusterSplitProbContainer='',
                            previousActsExtension=None):
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

        # Athena Track
        if flags.Tracking.ActiveConfig.doAthenaTrack:
            if (flags.Tracking.ActiveConfig.extension in
                ["Conversion", "ActsValidateConversionSeeds"]):
                from InDetConfig.SiSPSeededTrackFinderConfig import (
                    ITkSiSPSeededTrackFinderROIConvCfg)
                acc.merge(ITkSiSPSeededTrackFinderROIConvCfg(
                    flags, TracksLocation=SiSPSeededTrackCollectionKey))
            else:                
                from InDetConfig.SiSPSeededTrackFinderConfig import (
                    ITkSiSPSeededTrackFinderCfg)
                acc.merge(ITkSiSPSeededTrackFinderCfg(
                    flags, TracksLocation=SiSPSeededTrackCollectionKey))
                
        # GNN Track
        if flags.Tracking.ActiveConfig.doGNNTrack:
            from InDetGNNTracking.InDetGNNTrackingConfig import GNNTrackMakerCfg
            acc.merge(GNNTrackMakerCfg(
                flags, TracksLocation=SiSPSeededTrackCollectionKey))

        # FPGA seed
        if flags.Tracking.ActiveConfig.doFPGASeed:
            if flags.Tracking.ActiveConfig.doFPGATrackSim:
                from FPGATrackSimConfTools import FPGATrackSimAnalysisConfig
                acc.merge(FPGATrackSimAnalysisConfig.FPGATrackSimSeedingCfg(flags)) 
            else:
                from EFTrackingFPGAPipeline.F150IntegrationConfig import FPGA150Pipeline
                acc.merge(FPGA150Pipeline(flags, runStandalone=False))
        
        # ACTS seed
        if flags.Tracking.ActiveConfig.doActsSeed:
            from ActsConfig.ActsSeedingConfig import ActsSeedingCfg
            acc.merge(ActsSeedingCfg(flags))

        # ACTS track
        if flags.Tracking.ActiveConfig.doActsTrack:
            from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
            acc.merge(ActsTrackFindingCfg(flags))

        # Convert Tracks Acts -> Athena (before ambi)
        if flags.Tracking.ActiveConfig.doActsToAthenaTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(
                flags,
                ACTSTracksLocation=f"{flags.Tracking.ActiveConfig.extension}Tracks",
                TracksLocation=SiSPSeededTrackCollectionKey))

        # Convert tracks Athena -> Acts (before ambi)
        if flags.Tracking.ActiveConfig.doAthenaToActsTrack:
            from ActsConfig.ActsEventCnvConfig import TrkToActsConvertorAlgCfg
            acc.merge(TrkToActsConvertorAlgCfg(
                flags,
                TrackContainerLocation=f"{flags.Tracking.ActiveConfig.extension}Tracks",
                TrackCollectionKeys=[SiSPSeededTrackCollectionKey]))


    runTruth = (flags.Tracking.ActiveConfig.doAthenaTrack or
                flags.Tracking.ActiveConfig.doActsToAthenaTrack or
                flags.Tracking.ActiveConfig.doGNNTrack)
    from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
    if flags.Tracking.doTruth and runTruth:
        acc.merge(ITkTrackTruthCfg(
            flags, Tracks=SiSPSeededTrackCollectionKey,
            DetailedTruth=SiSPSeededTrackCollectionKey+"DetailedTruth",
            TracksTruth=SiSPSeededTrackCollectionKey+"TruthCollection"))
        
    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------

    runCopyAlg = ((flags.Tracking.doITkFastTracking and
                   flags.Tracking.ActiveConfig.doAthenaTrack) or
                  (flags.Tracking.ActiveConfig.doGNNTrack and
                   not flags.Tracking.GNN.doAmbiResolution))

    if runCopyAlg:
        from TrkConfig.TrkCollectionAliasAlgConfig import CopyAlgForAmbiCfg
        acc.merge(CopyAlgForAmbiCfg(
            flags, "ITkCopyAlgForAmbi"+flags.Tracking.ActiveConfig.extension,
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
                flags, SiSPSeededTrackCollectionKey=SiSPSeededTrackCollectionKey,
                ClusterSplitProbContainer=ClusterSplitProbContainer))
            
            acc.merge(ITkTrkAmbiguitySolverCfg(
                flags, ResolvedTrackCollectionKey=ResolvedTrackCollectionKey))

        # If we run Acts tracking we may want Acts ambi, depending on the flag
        if flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            # Schedule ACTS ambi. resolution and eventually the track convertions  
            from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
            acc.merge(ActsAmbiguityResolutionCfg(flags))

            from ActsConfig.ActsPrdAssociationConfig import ActsPrdAssociationAlgCfg
            acc.merge(ActsPrdAssociationAlgCfg(
                flags, name = f'{flags.Tracking.ActiveConfig.extension}PrdAssociationAlg',
                previousActsExtension=previousActsExtension))

        if flags.Tracking.ActiveConfig.doActsToAthenaResolvedTrack:
            from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
            acc.merge(ActsToTrkConvertorAlgCfg(
                flags,
                ACTSTracksLocation=f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks",
                TracksLocation=ResolvedTrackCollectionKey))

    runTruth = (flags.Tracking.ActiveConfig.doAthenaTrack or
                flags.Tracking.ActiveConfig.doAthenaAmbiguityResolution or
                (flags.Tracking.ActiveConfig.doGNNTrack and
                 not flags.Tracking.GNN.doAmbiResolution))
    if flags.Tracking.doTruth and runTruth:
        acc.merge(ITkTrackTruthCfg(
            flags, Tracks=ResolvedTrackCollectionKey,
            DetailedTruth=ResolvedTrackCollectionKey+"DetailedTruth",
            TracksTruth=ResolvedTrackCollectionKey+"TruthCollection"))

    if flags.Tracking.ActiveConfig.doActsTrack and flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import (
            ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg)
        acts_tracks = (f"{flags.Tracking.ActiveConfig.extension}Tracks"
                       if not flags.Acts.doAmbiguityResolution else
                       f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks")
        acc.merge(ActsTrackToTruthAssociationAlgCfg(
            flags, name=f"{acts_tracks}TrackToTruthAssociationAlg",
            ACTSTracksLocation=acts_tracks,
            AssociationMapOut=acts_tracks+"ToTruthParticleAssociation"))

        acc.merge(ActsTrackFindingValidationAlgCfg(
            flags, name=f"{acts_tracks}TrackFindingValidationAlg",
            TrackToTruthAssociationMap=acts_tracks+"ToTruthParticleAssociation"))


    return acc
