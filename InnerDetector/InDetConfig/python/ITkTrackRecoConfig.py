# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format
from TrkConfig.TrackingPassFlags import printActiveConfig
from AthenaCommon.Constants import WARNING, INFO

_flags_set = []  # For caching
_extensions_list = [] # For caching, possible legacy / validate Passes/Configurations
_actsExtensions  = ['Acts', 'ActsFast', 'ActsConversion', 'ActsLargeRadius', 'ActsLowPt'] # Possible Acts Alone Passes/Configurations
_outputExtensions  = [] # Passes/Configurations to be passed to the output job option

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
        TrackingComponent.ActsValidateConversionSeeds : "ActsValidateConversionSeeds",
        TrackingComponent.ActsValidateLargeRadiusSeeds: "ActsValidateLargeRadiusSeeds",
        TrackingComponent.ActsValidateTracks : "ActsValidateTracks",
        TrackingComponent.ActsValidateAmbiguityResolution : "ActsValidateAmbiguityResolution",
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

    # Acts Fast Pass
    if TrackingComponent.ActsFastChain in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsFastPass")]
        
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

    # Acts Large Radius Pass
    if flags.Acts.doLargeRadius:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLargeRadiusPass")]
        
    # Acts Conversion Pass
    if flags.Detector.EnableCalo and flags.Acts.doITkConversion and \
       TrackingComponent.ActsValidateConversionSeeds not in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsConversionPass")]

    # Acts Low Pt Pass
    if flags.Acts.doLowPt:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkActsLowPtPass")]
        
    # Acts Validation Passes
    for [configuration, key] in validation_configurations.items():
        if configuration in flags.Tracking.recoChain:
            toAdd = eval(f"flags.cloneAndReplace('Tracking.ActiveConfig', 'Tracking.ITk{key}Pass')")
            flags_set += [toAdd]

    # LRT
    if flags.Tracking.doLargeD0:
        if flags.Tracking.useITkFTF:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkFTFLargeD0Pass")
        elif flags.Tracking.doITkFastTracking:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0FastPass")
        else:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ITkLargeD0Pass")
        flags_set += [flagsLRT]

    ## FPGA pass 
    if TrackingComponent.FPGAChain in flags.Tracking.recoChain:
        flags_set += [flags.cloneAndReplace(
            "Tracking.ActiveConfig",
            "Tracking.ITkFPGAPass")]

    # Photon conversion tracking reco
    if flags.Detector.EnableCalo and flags.Tracking.doITkConversion:
        flagsConv = flags.cloneAndReplace("Tracking.ActiveConfig",
                                          "Tracking.ITkConversionPass")
        flags_set += [flagsConv]

    # LowPt
    if flags.Tracking.doLowPt:
        flagsLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.ITkLowPtPass")
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
        AssociationMapName = ("PRDtoTrackMapMerge_Resolved" +
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
    if flags.Tracking.doTruth and extension not in _actsExtensions:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    # Create track particles from all the different track collections
    # We have different algorithms depending on the EDM being used
    if extension not in _actsExtensions:
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
                                                       ACTSTracksLocation=[TrackContainer],
                                                       TrackParticlesOutKey=f'InDet{prefix}TrackParticles'))
        
        if flags.Tracking.doTruth :
            from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
            result.merge(ActsTrackParticleTruthDecorationAlgCfg(flags,
                                                                name=f'{TrackContainer}ParticleTruthDecorationAlg',
                                                                TrackToTruthAssociationMaps = [f'{TrackContainer}ToTruthParticleAssociation'],
                                                                TrackParticleContainerName = f'InDet{prefix}TrackParticles'
                                                                ))
    return result


# Returns CA + ClusterSplitProbContainer
def ITkTrackRecoPassCfg(flags,
                        previousActsExtension: str = None,
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
    if extension in _actsExtensions:
        TrackContainer = extension + "ResolvedTracks"
    if doTrackOverlay and extension == "Conversion":
        TrackContainer = flags.Overlay.SigPrefix + TrackContainer

    # This is the track collection BEFORE the ambiguity resolution
    SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"
    # For ACTS the name is totally different
    if  extension in _actsExtensions:
        SiSPSeededTracks = extension + "Tracks"
        
    # This performs track finding
    from InDetConfig.ITkTrackingSiPatternConfig import ITkTrackingSiPatternCfg
    result.merge(ITkTrackingSiPatternCfg(
        flags,
        previousActsExtension=previousActsExtension,
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
        if extension in _actsExtensions and not flags.Tracking.ActiveConfig.doActsAmbiguityResolution:
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
        if extension not in _actsExtensions:
            InputCombinedITkTracks += [TrackContainer]
        else:
            InputCombinedActsTracks += [TrackContainer]

    # This is only used in this same function for the Track-PRD association
    # Not yet supported for ACTS tracks
    if extension not in _actsExtensions:
        InputExtendedITkTracks += [TrackContainer]
        
    return result, ClusterSplitProbContainer


def ITkActsTrackFinalCfg(flags,
                         InputCombinedITkTracks: list[str] = None,
                         ActsTrackContainerName="InDetActsTrackParticles") -> ComponentAccumulator:
    # Inputs must not be None
    assert InputCombinedITkTracks is not None and isinstance(InputCombinedITkTracks, list)

    acc = ComponentAccumulator()
    if len(InputCombinedITkTracks) == 0:
        return acc
    
    # Schedule Track particle creation
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, "ActsCombinedTrackToTrackParticleCnvAlg",
                                                ACTSTracksLocation=InputCombinedITkTracks,
                                                TrackParticlesOutKey=ActsTrackContainerName))
    if flags.Tracking.doTruth :
        from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
        track_to_truth_maps=[]
        for track_container in InputCombinedITkTracks :
            track_to_truth_maps += [f'{track_container}ToTruthParticleAssociation']
        # note: suppress stat dumps from ActsTrackParticleTruthDecorationAlg if there is only
        #       a single input collection, because it duplicates in that case the output of
        #       the TrackFindingValidationAlg
        acc.merge(ActsTrackParticleTruthDecorationAlgCfg(
                     flags,
                     name=f'{ActsTrackContainerName}TruthDecorationAlg',
                     TrackToTruthAssociationMaps = track_to_truth_maps,
                     TrackParticleContainerName = ActsTrackContainerName,
                     OutputLevel=WARNING              if len(InputCombinedITkTracks)==1 else INFO,
                     ComputeTrackRecoEfficiency=False if len(InputCombinedITkTracks)==1 else True
                     ))
    return acc

def ITkTrackFinalCfg(flags,
                     InputCombinedITkTracks: list[str] = None,
                     StatTrackCollections: list[str] = None,
                     StatTrackTruthCollections: list[str] = None):
    # None of the input collection is supposed to be None
    assert InputCombinedITkTracks is not None and isinstance(InputCombinedITkTracks, list)
    assert StatTrackCollections is not None and isinstance(StatTrackCollections, list)
    assert StatTrackTruthCollections is not None and isinstance(StatTrackTruthCollections, list)

    # If there are no tracks return directly
    result = ComponentAccumulator()
    if len(InputCombinedITkTracks) == 0:
        return result
    
    if hasattr(flags.TrackOverlay, "ActiveConfig"):
       doTrackOverlay = getattr(flags.TrackOverlay.ActiveConfig, "doTrackOverlay", None)
    else:
       doTrackOverlay = flags.Overlay.doTrackOverlay

    TrackContainer = "CombinedITkTracks"
    if doTrackOverlay:
        #schedule merge to combine signal and background tracks
        InputCombinedITkTracks += [flags.Overlay.BkgPrefix + TrackContainer]
    
    from TrkConfig.TrkConfigFlags import TrackingComponent
    doGNNWithoutAmbiReso = (TrackingComponent.GNNChain in flags.Tracking.recoChain and (not flags.Tracking.GNN.doAmbiResolution))
    skipClusterMerge = doGNNWithoutAmbiReso or flags.Tracking.doITkFastTracking
    # This merges track collections
    from TrkConfig.TrkTrackCollectionMergerConfig import (
        ITkTrackCollectionMergerAlgCfg)
    result.merge(ITkTrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedITkTracks,
        OutputCombinedTracks=TrackContainer,
        AssociationMapName=(
            "" if skipClusterMerge else
            f"PRDtoTrackMapMerge_{TrackContainer}")))

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
            "" if skipClusterMerge else
            splitProbName),
        AssociationMapName=(
            "" if skipClusterMerge else
            f"PRDtoTrackMapMerge_{TrackContainer}"),
        isActsAmbi = 'ActsValidateResolvedTracks' in splitProbName or \
        'ActsValidateAmbiguityResolution' in splitProbName or \
        'ActsValidateScoreBasedAmbiguityResolution' in splitProbName or \
        'ActsConversion' in splitProbName or \
        'ActsLargeRadius' in splitProbName or \
        'ActsLowPt' in splitProbName or \
        ('Acts' in  splitProbName and 'Validate' not in splitProbName) ))

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


def ITkActsExtendedPRDInfoCfg(flags):
    result = ComponentAccumulator()

    #Add the truth origin to the truth particles
    from InDetConfig.InDetPrepRawDataToxAODConfig import ITkActsPixelPrepDataToxAODCfg
    result.merge(ITkActsPixelPrepDataToxAODCfg(flags))

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
            "" if flags.Tracking.doITkFastTracking else
            ITkClusterSplitProbabilityContainerName(flags))))
    result.merge(StripPrepDataToxAODCfg(flags))

    from DerivationFrameworkInDet.InDetToolsConfig import (
        ITkTSOS_CommonKernelCfg)
    result.merge(ITkTSOS_CommonKernelCfg(flags))

    if flags.Tracking.doStoreSiSPSeededTracks:
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e=='') or
            flags.Tracking[f"ITk{e}Pass"].storeSiSPSeededTracks ]
        from DerivationFrameworkInDet.InDetToolsConfig import (
            ITkSiSPTSOS_CommonKernelCfg)
        result.merge(ITkSiSPTSOS_CommonKernelCfg(flags, listOfExtensions = listOfExtensionsRequesting))

    if flags.Input.isMC:
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e=='') or
            (flags.Tracking[f"ITk{e}Pass"].storeSiSPSeededTracks and
             flags.Tracking[f"ITk{e}Pass"].storeSeparateContainer) ]
        from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
            ITkPhysHitDecoratorAlgCfg)
        for extension in listOfExtensionsRequesting:
            result.merge(ITkPhysHitDecoratorAlgCfg(
                flags,
                name=f"ITkPhysHit{extension}DecoratorAlg",
                TrackParticleContainerName=f"InDet{extension}TrackParticles"))

    return result


##############################################################################
#####################     Main ITk tracking config       #####################
##############################################################################


def ITkTrackRecoCfg(flags) -> ComponentAccumulator:
    """Configures complete ITk tracking """

    from TrkConfig.TrkConfigFlags import ITkPrimaryPassConfig
    if flags.Tracking.ITkPrimaryPassConfig in [ITkPrimaryPassConfig.Acts, ITkPrimaryPassConfig.ActsFast, ITkPrimaryPassConfig.ActsHeavyIon]:
        from InDetConfig.ITkActsTrackRecoConfig import ITkActsTrackRecoCfg
        return ITkActsTrackRecoCfg(flags)

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
    # Record previous ACTS extension
    previousActsExtension = None
    
    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg

    for current_flags in flags_set:
        printActiveConfig(current_flags)

        
        extension = current_flags.Tracking.ActiveConfig.extension
        if extension not in _actsExtensions:
            _extensions_list.append(extension)

        # Add the extension to the output job option
        _outputExtensions.append(extension)

        # Data Preparation
        # According to the tracking pass we have different data preparation 
        # sequences. We may have:
        # (1) Full Athena data preparation  
        # (2) Full Acts data preparation 
        # (3) Hybrid configurations with EDM converters
        result.merge(ITkRecPreProcessingSiliconCfg(current_flags,
                                                   previousActsExtension=previousActsExtension))

        # Track Reconstruction
        # This includes track finding and ambiguity resolution
        # The output is the component accumulator to be added to the sequence
        # and the name of the cluster split prob container that is used for
        # removing measurements used by previous passes
        # This last object will also assure the proper sequence of the tracking passes
        # since it will create a data dependency from the prevous pass
        acc, ClusterSplitProbContainer = ITkTrackRecoPassCfg(
            current_flags,
            previousActsExtension,
            InputCombinedITkTracks=InputCombinedITkTracks,
            InputCombinedActsTracks=InputCombinedActsTracks,
            InputExtendedITkTracks=InputExtendedITkTracks,
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

        # Store ACTS extension
        if 'Acts' in extension:
            previousActsExtension = extension

    # This merges the track collection in InputCombinedITkTracks
    # and creates a track particle collection from that
    if InputCombinedITkTracks:

        result.merge(
            ITkTrackFinalCfg(flags,
                             InputCombinedITkTracks=InputCombinedITkTracks,
                             StatTrackCollections=StatTrackCollections,
                             StatTrackTruthCollections=StatTrackTruthCollections))
        

    
        
    # Now handle ACTS tracks instead if present in the event
    if InputCombinedActsTracks:

        # The name of the Acts xAOD container name depends on what
        # workflow has been executed, which we get by checking
        # the size of the track containers.
                
        ActsTrackContainerName = "InDetTrackParticles" if not InputCombinedITkTracks else "InDetActsTrackParticles"
        ActsPrimaryVertices    = "PrimaryVertices" if not InputCombinedITkTracks else "ActsPrimaryVertices"

        result.merge(ITkActsTrackFinalCfg(flags,
                                          InputCombinedITkTracks=InputCombinedActsTracks,
                                          ActsTrackContainerName=ActsTrackContainerName))

    # Perform vertex finding
    if flags.Tracking.doVertexFinding:
        
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg

        # Schedule the usual vertex finding for Athena workflow(s)
        # ONLY schedule this if there are Athena Legacy Track collections
        if InputCombinedITkTracks:
            result.merge(primaryVertexFindingCfg(flags))

        # Schedule the same vertex finding for Acts workflow(s)
        # For now this is separate from the Athena counterpart, but in the
        # end the difference will not be needed anymore
        # ONLY schedule this if there are ACTS Track collections
        if InputCombinedActsTracks:
            result.merge(primaryVertexFindingCfg(flags,
                                                 name="ActsPriVxFinderAlg",
                                                 TracksName=ActsTrackContainerName,
                                                 vxCandidatesOutputName=ActsPrimaryVertices))

    # Post Processing
    # This is mainly for validation support
    print("-------- POST PROCESSING --------")
    for current_flags in flags_set:
        extension = current_flags.Tracking.ActiveConfig.extension
        print(f"- Running post-processing for extension: {extension}")

        # Persistify the Seeds
        # This is done for the InDet EDM, while for ACTS seeds the persistification
        # support is done while scheduling the seeding algorithms
        # This means that here we only care about the legacy seeds, that are
        # produced whem the legacy Track Finding is scheduled
        # - flags.Tracking.ActiveConfig.doAthenaTrack
        # This also covers the case of the Acts->InDet Seed conversion, since that
        # runs the legacy track finding as well
        #
        # At the end of this we have a track collection (segments from the seeds)
        # and the corresponding track particle collection
        if current_flags.Tracking.ActiveConfig.doAthenaTrack:
            if current_flags.Tracking.doStoreTrackSeeds:
                from InDetConfig.ITkPersistificationConfig import ITkTrackSeedsFinalCfg
                result.merge(ITkTrackSeedsFinalCfg(current_flags))
        
        # Persistify Track from Track Finding
        # Currently this is only possible for Trk Tracks
        # For legacy tracking passes the CKF Trk Tracks are always produced
        # - flags.Tracking.ActiveConfig.doAthenaTrack
        # For hybrid ACTS-Athena tracking passes they are produced only after Track EDM conversion
        # - flags.Tracking.ActiveConfig.doActsToAthenaTrack
        if current_flags.Tracking.ActiveConfig.doAthenaTrack or current_flags.Tracking.ActiveConfig.doActsToAthenaTrack:
            if current_flags.Tracking.doStoreSiSPSeededTracks:
                from InDetConfig.ITkPersistificationConfig import ITkSiSPSeededTracksFinalCfg
                result.merge(ITkSiSPSeededTracksFinalCfg(current_flags))

    if flags.Tracking.doStats:
        if _extensions_list:
            result.merge(ITkStatsCfg(
                flags_set[0], # Use cuts from primary pass
                StatTrackCollections=StatTrackCollections,
                StatTrackTruthCollections=StatTrackTruthCollections))


    ## ACTS Specific write PRDInfo
    if flags.Tracking.writeExtendedSi_PRDInfo:
        if _extensions_list:
            result.merge(ITkExtendedPRDInfoCfg(flags))

            #Acts algorithm
        else:
            result.merge(ITkActsExtendedPRDInfoCfg(flags))
            
    # output
    from InDetConfig.ITkTrackOutputConfig import ITkTrackRecoOutputCfg
    result.merge(ITkTrackRecoOutputCfg(flags, _outputExtensions))
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
