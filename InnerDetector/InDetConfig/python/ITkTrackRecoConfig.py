# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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
        TrackingComponent.ValidateActsClusters : "ValidateActsClusters",
        TrackingComponent.ValidateActsSpacePoints : "ValidateActsSpacePoints",
        TrackingComponent.ValidateActsSeeds : "ValidateActsSeeds",
        TrackingComponent.ValidateActsTracks : "ValidateActsTracks",
        TrackingComponent.ValidateActsAmbiguityResolution : "ValidateActsAmbiguityResolution",
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


def ITkStoreTrackSeparateContainerCfg(flags, TrackContainer="",
                                      ClusterSplitProbContainer=""):
    result = ComponentAccumulator()
    extension = flags.Tracking.ActiveConfig.extension

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

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

    return result


# Returns CA + ClusterSplitProbContainer
def ITkTrackRecoPassCfg(flags, extension="",
                        InputCombinedITkTracks=None,
                        InputExtendedITkTracks=None,
                        StatTrackCollections=None,
                        StatTrackTruthCollections=None,
                        ClusterSplitProbContainer=""):
    if InputCombinedITkTracks is None:
        InputCombinedITkTracks = []
    if InputExtendedITkTracks is None:
        InputExtendedITkTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    result = ComponentAccumulator()

    TrackContainer = "Resolved" + extension + "Tracks"
    SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"

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

    if flags.Tracking.ActiveConfig.storeSeparateContainer:
        result.merge(ITkStoreTrackSeparateContainerCfg(
            flags,
            TrackContainer=TrackContainer,
            ClusterSplitProbContainer=ClusterSplitProbContainer))
    else:
        if extension not in ['Acts', 'ActsConversion']:
            ClusterSplitProbContainer = (
                "ITkAmbiguityProcessorSplitProb" + extension)
            InputCombinedITkTracks += [TrackContainer]

    if extension not in ['Acts', 'ActsConversion']:
        InputExtendedITkTracks += [TrackContainer]
        
    return result, ClusterSplitProbContainer


def ITkTrackFinalCfg(flags,
                     InputCombinedITkTracks=None,
                     StatTrackCollections=None,
                     StatTrackTruthCollections=None):
    if InputCombinedITkTracks is None:
        InputCombinedITkTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    result = ComponentAccumulator()

    TrackContainer = "CombinedITkTracks"

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

    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    result.merge(ITkTrackParticleCnvAlgCfg(
        flags,
        ClusterSplitProbabilityName=(
            "" if flags.Tracking.doITkFastTracking else
            splitProbName),
        AssociationMapName=(
            "" if flags.Tracking.doITkFastTracking else
            f"PRDtoTrackMap{TrackContainer}"),
        isActsAmbi = 'ValidateActsResolvedTracks' in splitProbName or \
        'ValidateActsAmbiguityResolution' in splitProbName or \
        'ActsConversion' in splitProbName or \
        ('Acts' in  splitProbName and 'Validate' not in splitProbName) ))

    return result


def ITkTrackSeedsFinalCfg(flags):
    result = ComponentAccumulator()

    # get list of extensions requesting track seeds.
    # Add always the Primary Pass.
    listOfExtensionsRequesting = [
        e for e in _extensions_list
        if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]

    for extension in listOfExtensionsRequesting:
        TrackContainer = "SiSPSeedSegments"+extension

        if flags.Tracking.doTruth:
            from InDetConfig.TrackTruthConfig import ITkTrackTruthCfg
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
        if (e=='' or flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks) ]

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


def ITkTrackRecoCfg(flags):
    """Configures complete ITk tracking """
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        # TODO: ITk BS providers
        raise RuntimeError("ByteStream inputs not supported")

    flags_set = CombinedTrackingPassFlagSets(flags)
    # Tracks to be ultimately merged in InDetTrackParticle collection
    InputCombinedITkTracks = []
    # Includes also tracks which end in standalone TrackParticle collections
    InputExtendedITkTracks = []
    ClusterSplitProbContainer = ""
    StatTrackCollections = []  # To be passed to the InDetRecStatistics alg
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

        # Track Reco
        acc, ClusterSplitProbContainer = ITkTrackRecoPassCfg(
            current_flags, extension=extension,
            InputCombinedITkTracks=InputCombinedITkTracks,
            InputExtendedITkTracks=InputExtendedITkTracks,
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

    result.merge(
        ITkTrackFinalCfg(flags,
                         InputCombinedITkTracks=InputCombinedITkTracks,
                         StatTrackCollections=StatTrackCollections,
                         StatTrackTruthCollections=StatTrackTruthCollections))

    if flags.Tracking.doStoreTrackSeeds:
        result.merge(ITkTrackSeedsFinalCfg(flags))

    if flags.Tracking.doStoreSiSPSeededTracks:
        result.merge(ITkSiSPSeededTracksFinalCfg(flags))

    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

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
