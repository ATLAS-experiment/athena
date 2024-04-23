# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format
from TrkConfig.TrackingPassFlags import printActiveConfig

_flags_set = []  # For caching
_extensions_list = [] # For caching

def CombinedTrackingPassFlagSets(flags):
    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass(es)
    from TrkConfig.TrkConfigFlags import TrackingComponent
    validation_configurations = {
        TrackingComponent.ActsValidateClusters : "ActsValidateClusters",
        TrackingComponent.ActsValidateSpacePoints : "ActsValidateSpacePoints",
        TrackingComponent.ActsValidateSeeds : "ActsValidateSeeds",
        TrackingComponent.ActsValidateTracks : "ActsValidateTracks",
        TrackingComponent.ActsValidateAmbiguityResolution : "ActsValidateAmbiguityResolution",
        TrackingComponent.BenchmarkSpot : "ActsBenchmarkSpot"
    }
    
    # Athena Pass
    if TrackingComponent.AthenaChain in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            f"Tracking.{flags.Tracking.ITkPrimaryPassConfig.value}Pass")]

    # Acts Pass
    if TrackingComponent.ActsChain in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsPass")]

    # Acts Heavy Ion Pass
    if TrackingComponent.ActsHeavyIon in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsHeavyIonPass")]
        
    # GNN pass
    if TrackingComponent.GNNChain in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkGNNPass")]

    # Acts Conversion Pass
    if flags.Detector.EnableCalo and flags.Acts.doITkConversion:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsConversionPass")]
        
    # Acts Validation Passes
    for [configuration, key] in validation_configurations.items():
        if configuration in flags.Tracking.recoChain:
            toAdd = eval(f"flags.cloneAndReplace('Tracking.ActiveConfig', 'Tracking.ITk{key}Pass')")
            flags_set += [toAdd]

    # LRT
    if flags.Tracking.doLargeD0:
        if flags.Tracking.doITkFastTracking:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0FastPass")
        else:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0Pass")
        flags_set += [flagsLRT]

    # Photon conversion tracking reco
    if flags.Detector.EnableCalo and flags.Tracking.doITkConversion:
        flagsConv = flags.cloneAndReplace("Tracking.ActiveConfig",
                                          "Tracking.ITkConversionPass")
        flags_set += [flagsConv]

    # LowPt
    if flags.Tracking.doLowPt:
        flagsLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.ITkLowPt")
        flags_set += [flagsLowPt]

    _flags_set = flags_set  # Put into cache

    return flags_set


def ITkClusterSplitProbabilityContainerName(flags):
    flags_set = CombinedTrackingPassFlagSets(flags)
    extension = flags_set[-1].Tracking.ActiveConfig.extension
    ClusterSplitProbContainer = "ITkAmbiguityProcessorSplitProb" + extension    
    return ClusterSplitProbContainer


def ITkStoreTrackSeparateContainerCfg(flags,
                                      TrackContainer: str ="",
                                      ClusterSplitProbContainer: str = "") -> ComponentAccumulator:
    result = ComponentAccumulator()
    extension = flags.Tracking.ActiveConfig.extension
    if hasattr(flags.TrackOverlay, "ActiveConfig"):
       doTrackOverlay = getattr(flags.TrackOverlay.ActiveConfig, "doTrackOverlay", None)
    else:
       doTrackOverlay = flags.Overlay.doTrackOverlay

    if doTrackOverlay:
        # schedule merger to combine signal and background tracks
        InputTracks = [flags.Overlay.SigPrefix+TrackContainer,
                       flags.Overlay.BkgPrefix+TrackContainer]
        AssociationMapName = ("PRDtoTrackMapResolved" +
                              extension + "Tracks")
        MergerOutputTracks = TrackContainer

        from TrkConfig.TrkTrackCollectionMergerConfig import TrackCollectionMergerAlgCfg
        result.merge(TrackCollectionMergerAlgCfg(
            flags,
            name="TrackCollectionMergerAlgCfg"+extension,
            InputCombinedTracks=InputTracks,
            OutputCombinedTracks=MergerOutputTracks,
            AssociationMapName=AssociationMapName))

    # Run truth, but only do this for non ACTS workflows
    if flags.Tracking.doTruth and extension not in ['Acts', 'ActsConversion']:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    # Create track particles from all the different track collections
    # We have different algorithms depending on the EDM being used
    if extension not in ['Acts', 'ActsConversion']:
        # Workflows that use Trk Tracks
        from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
        result.merge(ITkTrackParticleCnvAlgCfg(
            flags,
            name=extension + "TrackParticleCnvAlg",
            TrackContainerName=TrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                "InDet" + extension + "TrackParticles"),
            ClusterSplitProbabilityName=(
                "" if flags.Tracking.doITkFastTracking else
                ClusterSplitProbContainer),
            AssociationMapName=""))
    else:
        # Workflows that use Acts Tracks
        from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
        # The following few lines will disappear once we have imposed a proper nomenclature for our algorithms and collection
        prefix = flags.Tracking.ActiveConfig.extension
        result.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, f"{prefix}ResolvedTrackToAltTrackParticleCnvAlg",
                                                       ACTSTracksLocation=TrackContainer,
                                                       TrackParticlesOutKey=f'{TrackContainer}ParticlesAlt'))
            
            
    return result


# Returns CA + ClusterSplitProbContainer
def ITkTrackRecoPassCfg(flags,
                        InputCombinedITkTracks: list[str] = None,
                        InputCombinedActsTracks: list[str] = None,
                        InputExtendedITkTracks: list[str] = None,
                        StatTrackCollections: list[str] = None,
                        StatTrackTruthCollections: list[str] = None,
                        ClusterSplitProbContainer: str = ""):
    # We use these lists to store the collections from all the tracking passes, thus keeping the history
    # of previous passes. None of these lists is allowed to be a None
    assert InputCombinedITkTracks is not None and isinstance(InputCombinedITkTracks, list)
    assert InputCombinedActsTracks is not None and isinstance(InputCombinedActsTracks, list)
    assert InputExtendedITkTracks is not None and isinstance(InputExtendedITkTracks, list)
    assert StatTrackCollections is not None and isinstance(StatTrackCollections, list)
    assert StatTrackTruthCollections is not None and isinstance(StatTrackTruthCollections ,list)

    # Get the tracking pass extension name
    extension = flags.Tracking.ActiveConfig.extension
    
    result = ComponentAccumulator()
    if hasattr(flags.TrackOverlay, "ActiveConfig"):
       doTrackOverlay = getattr(flags.TrackOverlay.ActiveConfig, "doTrackOverlay", None)
    else:
       doTrackOverlay = flags.Overlay.doTrackOverlay

    # Define collection name(s)
    # This is the track collection AFTER the ambiguity resolution
    TrackContainer = "Resolved" + extension + "Tracks"
    # For Acts we have another convention, with the extention as the first element in the name
    if extension in ['Acts', 'ActsConversion']:
        TrackContainer = extension + "ResolvedTracks"
    if doTrackOverlay and extension == "Conversion":
        TrackContainer = flags.Overlay.SigPrefix + TrackContainer

    # This is the track collection BEFORE the ambiguity resolution
    SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"
    # For ACTS the name is totally different
    if  extension in ['Acts', 'ActsConversion']:
        SiSPSeededTracks = extension + "Tracks"
        
    # This performs track finding
    from InDetConfig.ITkTrackingSiPatternConfig import ITkTrackingSiPatternCfg
    result.merge(ITkTrackingSiPatternCfg(
        flags,
        InputCollections=InputExtendedITkTracks,
        ResolvedTrackCollectionKey=TrackContainer,
        SiSPSeededTrackCollectionKey=SiSPSeededTracks,
        ClusterSplitProbContainer=ClusterSplitProbContainer))
    StatTrackCollections += [SiSPSeededTracks, TrackContainer]
    StatTrackTruthCollections += [SiSPSeededTracks+"TruthCollection",
                                  TrackContainer+"TruthCollection"]
    
    if doTrackOverlay and extension == "Conversion":
        TrackContainer = "Resolved" + extension + "Tracks"
        result.merge(ITkStoreTrackSeparateContainerCfg(
            flags,
            TrackContainer=TrackContainer,
            ClusterSplitProbContainer=ClusterSplitProbContainer))

    if flags.Tracking.ActiveConfig.storeSeparateContainer:
        # If we do not want the track collection to be merged with another collection
        # then we immediately create the track particles from it
        # This happens inside ITkStoreTrackSeparateContainerCfg

        # Track container, for ACTS workflow, depends on whether we activated the ambiguity resolution or not
        inputTrack = TrackContainer
        if extension in ['Acts', 'ActsConversion'] and not flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
            inputTrack = SiSPSeededTracks

        result.merge(ITkStoreTrackSeparateContainerCfg(
            flags,
            TrackContainer=inputTrack,
            ClusterSplitProbContainer=ClusterSplitProbContainer))
    else:
        # ClusterSplitProbContainer is used for removing measurements used in previous passes
        # For ACTS this is still not possible, TO BE IMPLEMENTED
        ClusterSplitProbContainer = (
            "ITkAmbiguityProcessorSplitProb" + extension)
        # Collect all the Trk Track collections to be then merged in a single big collection
        # Merging will be done later, and after that we create track particles from the merged collection
        if extension not in ['Acts', 'ActsConversion']:
            InputCombinedITkTracks += [TrackContainer]
        else:
            InputCombinedActsTracks += [TrackContainer]

    # This is only used in this same function for the Track-PRD association
    # Not yet supported for ACTS tracks
    if extension not in ['Acts', 'ActsConversion']:
        InputExtendedITkTracks += [TrackContainer]
        
    return result, ClusterSplitProbContainer


def ITkActsTrackFinalCfg(flags,
                         InputCombinedITkTracks: list[str] = None) -> ComponentAccumulator:
    # Inputs must not be None
    assert InputCombinedITkTracks is not None and isinstance(InputCombinedITkTracks, list)

    acc = ComponentAccumulator()
    if len(InputCombinedITkTracks) == 0:
        return acc
    
    # Schedule track merger
    mergeTrackContainer = "ActsCombinedTracks"
    from ActsConfig.ActsTrackFindingConfig import ActsTrackMergerAlgCfg
    acc.merge(ActsTrackMergerAlgCfg(flags,
                                    InputTrackCollections = InputCombinedITkTracks,
                                    OutputTrackCollection = mergeTrackContainer))

    # Schedule Track particle creation
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, "ActsCombinedTrackToAltTrackParticleCnvAlg",
                                                ACTSTracksLocation=mergeTrackContainer,
                                                TrackParticlesOutKey=f'{mergeTrackContainer}ParticlesAlt'))
    
    
    return acc

def ITkTrackFinalCfg(flags,
                     InputCombinedITkTracks: list[str] = None,
                     StatTrackCollections: list[str] = None,
                     StatTrackTruthCollections: list[str] = None):
    # None of the input collection is supposed to be None
    assert InputCombinedITkTracks is not None and isinstance(InputCombinedITkTracks, list)
    assert StatTrackCollections is not None and isinstance(StatTrackCollections, list)
    assert StatTrackTruthCollections is not None and isinstance(StatTrackTruthCollections, list)

    result = ComponentAccumulator()
    if hasattr(flags.TrackOverlay, "ActiveConfig"):
       doTrackOverlay = getattr(flags.TrackOverlay.ActiveConfig, "doTrackOverlay", None)
    else:
       doTrackOverlay = flags.Overlay.doTrackOverlay

    TrackContainer = "CombinedITkTracks"
    if doTrackOverlay:
        #schedule merge to combine signal and background tracks
        InputCombinedITkTracks += [flags.Overlay.BkgPrefix + TrackContainer]

    # This merges track collections
    from TrkConfig.TrkTrackCollectionMergerConfig import (
        ITkTrackCollectionMergerAlgCfg)
    result.merge(ITkTrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedITkTracks,
        OutputCombinedTracks=TrackContainer,
        AssociationMapName=(
            "" if flags.Tracking.doITkFastTracking else
            f"PRDtoTrackMap{TrackContainer}")))

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=f"{TrackContainer}DetailedTruth",
            TracksTruth=f"{TrackContainer}TruthCollection"))

    StatTrackCollections += [TrackContainer]
    StatTrackTruthCollections += [f"{TrackContainer}TruthCollection"]

    if flags.Tracking.doSlimming:
        from TrkConfig.TrkTrackSlimmerConfig import TrackSlimmerCfg
        result.merge(TrackSlimmerCfg(
            flags,
            TrackLocation=[TrackContainer]))

    splitProbName = ITkClusterSplitProbabilityContainerName(flags)

    # This creates track particles
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    result.merge(ITkTrackParticleCnvAlgCfg(
        flags,
        ClusterSplitProbabilityName=(
            "" if flags.Tracking.doITkFastTracking else
            splitProbName),
        AssociationMapName=(
            "" if flags.Tracking.doITkFastTracking else
            f"PRDtoTrackMap{TrackContainer}"),
        isActsAmbi = 'ActsValidateResolvedTracks' in splitProbName or \
        'ActsValidateAmbiguityResolution' in splitProbName or \
        'ActsConversion' in splitProbName or \
        ('Acts' in  splitProbName and 'Validate' not in splitProbName) ))

    return result


def ITkTrackSeedsFinalCfg(flags):
    result = ComponentAccumulator()

    # get list of extensions requesting track seeds.
    # Add always the Primary Pass.
    listOfExtensionsRequesting = [
        e for e in _extensions_list
        if (e == '' or flags.Tracking.__getattr__(f"ITk{e}Pass").storeTrackSeeds) ]

    for extension in listOfExtensionsRequesting:
        TrackContainer = "SiSPSeedSegments"+extension

        if flags.Tracking.doTruth:
            from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
            result.merge(ITkTrackTruthCfg(
                flags,
                Tracks=TrackContainer,
                DetailedTruth=f"{TrackContainer}DetailedTruth",
                TracksTruth=f"{TrackContainer}TruthCollection"))

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            ITkTrackParticleCnvAlgCfg)
        result.merge(ITkTrackParticleCnvAlgCfg(
            flags,
            name=f"{TrackContainer}CnvAlg",
            TrackContainerName=TrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                f"{TrackContainer}TrackParticles")))

    return result


def ITkSiSPSeededTracksFinalCfg(flags):
    result = ComponentAccumulator()

    # get list of extensions requesting track candidates.
    # Add always the Primary Pass.
    listOfExtensionsRequesting = [
        e for e in _extensions_list
        if (e=='' or flags.Tracking.__getattr__(f"ITk{e}Pass").storeSiSPSeededTracks) ]

    for extension in listOfExtensionsRequesting:
        AssociationMapNameKey="PRDtoTrackMapCombinedITkTracks"
        if not (extension == ''):
            AssociationMapNameKey = f"ITkPRDtoTrackMap{extension}"

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            ITkTrackParticleCnvAlgCfg)
        result.merge(ITkTrackParticleCnvAlgCfg(
            flags,
            name = f"SiSPSeededTracks{extension}CnvAlg",
            TrackContainerName = f"SiSPSeeded{extension}Tracks",
            xAODTrackParticlesFromTracksContainerName=(
                f"SiSPSeededTracks{extension}TrackParticles"),
            AssociationMapName=AssociationMapNameKey))

    return result


def ITkStatsCfg(flags, StatTrackCollections=None,
                  StatTrackTruthCollections=None):
    result = ComponentAccumulator()

    from InDetConfig.InDetRecStatisticsConfig import (
        ITkRecStatisticsAlgCfg)
    result.merge(ITkRecStatisticsAlgCfg(
        flags,
        TrackCollectionKeys=StatTrackCollections,
        TrackTruthCollectionKeys=(
            StatTrackTruthCollections if flags.Tracking.doTruth else [])))

    if flags.Tracking.doTruth:
        from InDetConfig.InDetTrackClusterAssValidationConfig import (
            ITkTrackClusterAssValidationCfg)
        result.merge(ITkTrackClusterAssValidationCfg(
            flags,
            TracksLocation=StatTrackCollections))

    return result


def ITkExtendedPRDInfoCfg(flags):
    result = ComponentAccumulator()

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            ITkPixelPrepDataToxAOD_ExtraTruthCfg as PixelPrepDataToxAODCfg,
            ITkStripPrepDataToxAOD_ExtraTruthCfg as StripPrepDataToxAODCfg)
    else:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            ITkPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg,
            ITkStripPrepDataToxAODCfg as StripPrepDataToxAODCfg)

    result.merge(PixelPrepDataToxAODCfg(
        flags,
        ClusterSplitProbabilityName=(
            ITkClusterSplitProbabilityContainerName(flags))))
    result.merge(StripPrepDataToxAODCfg(flags))

    from DerivationFrameworkInDet.InDetToolsConfig import (
        ITkTSOS_CommonKernelCfg)
    result.merge(ITkTSOS_CommonKernelCfg(flags))

    if flags.Tracking.doStoreSiSPSeededTracks:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            ITkSiSPTSOS_CommonKernelCfg)
        result.merge(ITkSiSPTSOS_CommonKernelCfg(flags))

    if flags.Input.isMC:
        from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
            ITkPhysHitDecoratorAlgCfg)
        result.merge(ITkPhysHitDecoratorAlgCfg(flags))

    return result


##############################################################################
#####################     Main ITk tracking config       #####################
##############################################################################


def ITkTrackRecoCfg(flags) -> ComponentAccumulator:
    """Configures complete ITk tracking """
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        # TODO: ITk BS providers
        raise RuntimeError("ByteStream inputs not supported")

    # Get all the requested tracking passes
    flags_set = CombinedTrackingPassFlagSets(flags)

    # Store the names of several collections from all the different passes
    # These collections will then be used for different purposes
    
    # Tracks to be ultimately merged in InDetTrackParticle collection
    InputCombinedITkTracks = []
    # Same but for ACTS collection
    InputCombinedActsTracks = []
    # Includes also tracks which end in standalone TrackParticle collections
    InputExtendedITkTracks = []
    # Cluster split prob container for measurement removal
    ClusterSplitProbContainer = ""
    # To be passed to the InDetRecStatistics alg
    StatTrackCollections = []
    StatTrackTruthCollections = []

    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg

    for current_flags in flags_set:
        printActiveConfig(current_flags)

        extension = current_flags.Tracking.ActiveConfig.extension
        _extensions_list.append(extension)

        # Data Preparation
        # According to the tracking pass we have different data preparation 
        # sequences. We may have:
        # (1) Full Athena data preparation  
        # (2) Full Acts data preparation 
        # (3) Hybrid configurations with EDM converters
        result.merge(ITkRecPreProcessingSiliconCfg(current_flags))

        # Track Reconstruction
        # This includes track finding and ambiguity resolution
        # The output is the component accumulator to be added to the sequence
        # and the name of the cluster split prob container that is used for
        # removing measurements used by previous passes
        # This last object will also assure the proper sequence of the tracking passes
        # since it will create a data dependency from the prevous pass
        acc, ClusterSplitProbContainer = ITkTrackRecoPassCfg(
            current_flags,
            InputCombinedITkTracks=InputCombinedITkTracks,
            InputCombinedActsTracks=InputCombinedActsTracks,
            InputExtendedITkTracks=InputExtendedITkTracks,
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

    # This merges the track collection in InputCombinedITkTracks
    # and creates a track particle collection from that
    result.merge(
        ITkTrackFinalCfg(flags,
                         InputCombinedITkTracks=InputCombinedITkTracks,
                         StatTrackCollections=StatTrackCollections,
                         StatTrackTruthCollections=StatTrackTruthCollections))

    # This will handle ACTS tracks instead
    result.merge(ITkActsTrackFinalCfg(flags,
                                      InputCombinedITkTracks=InputCombinedActsTracks))

    # Store some collections for persistification
    # Used for validation and studies
    if flags.Tracking.doStoreTrackSeeds:
        result.merge(ITkTrackSeedsFinalCfg(flags))

    if flags.Tracking.doStoreSiSPSeededTracks:
        result.merge(ITkSiSPSeededTracksFinalCfg(flags))

    # Perform vertex finding
    if flags.Tracking.doVertexFinding:
        # Schedule the usual vertex finding for Athena workflow(s)
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

        # Schedule the same vertex finding for Acts workflow(s)
        # For now this is separate from the Athena counterpart, but in the
        # end the difference will not be needed anymore
        # ONLY schedule this if there are ACTS Track collections
        if InputCombinedActsTracks:
            result.merge(primaryVertexFindingCfg(flags,
                                                 name="ActsPriVxFinderAlg",
                                                 TracksName="ActsCombinedTracksParticlesAlt",
                                                 vxCandidatesOutputName="ActsPrimaryVertices"))


    if flags.Tracking.doStats:
        result.merge(ITkStatsCfg(
            flags_set[0], # Use cuts from primary pass
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections))

    if flags.Tracking.writeExtendedSi_PRDInfo:
        result.merge(ITkExtendedPRDInfoCfg(flags))


    # output
    from InDetConfig.ITkTrackOutputConfig import ITkTrackRecoOutputCfg
    result.merge(ITkTrackRecoOutputCfg(flags, _extensions_list))
    result.printConfig(withDetails = False, summariseProps = False)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN4

    import sys
    if "--doFTF" in sys.argv:
       flags.Tracking.useITkFTF = True
       flags.Tracking.doITkFastTracking = True

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    top_acc.merge(ITkTrackRecoCfg(flags))

    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("ITkTrackReco.pkl", "wb"))

    if "--norun" not in sys.argv:
        sc = top_acc.run(5)
        if sc.isFailure():
            sys.exit(-1)
