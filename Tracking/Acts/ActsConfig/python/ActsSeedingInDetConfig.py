# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsUtilities import extractChildKwargs
from ActsInterop import UnitConstants as ActsUnits
import AthenaCommon.SystemOfUnits as GaudiUnits

# ACTS tools
def ActsInDetPixelSeedingToolCfg(flags,
                            name: str = "ActsInDetPixelSeedingTool",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## ACTS seeding tool configurations for Run 3 InnerDetector Pixel

    kwargs.setdefault("numSeedIncrement" , float("inf")) 
    kwargs.setdefault("deltaZMax" , float("inf"))
    kwargs.setdefault("maxPtScattering", float("inf"))
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("rMax", 280. * ActsUnits.mm)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT / GaudiUnits.GeV * ActsUnits.GeV)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpact / GaudiUnits.mm * ActsUnits.mm)
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("rRangeMiddleSP", [
        [0,0],
        [0,0],
        [0,0],
        [20, 260],
        [20, 260],
        [40, 260],
        [20, 260],
        [20, 260],
        [0,0],
        [0,0],
        [0, 0]])

    kwargs.setdefault("cotThetaMax" , 7.40626311) 
    kwargs.setdefault("zMax", 2800. * ActsUnits.mm) 
    kwargs.setdefault("zMin", -2800. * ActsUnits.mm)
    
    kwargs.setdefault("deltaRMin" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMax" , 270. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 270. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinTopSP" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 270. * ActsUnits.mm)
    kwargs.setdefault("gridRMax" , kwargs['rMax'])
    kwargs.setdefault("phiBinDeflectionCoverage" , 1) 
    kwargs.setdefault("maxPhiBins" , 200) 
    kwargs.setdefault("sigmaScattering", 5.0) 
    kwargs.setdefault("radLengthPerSeed", 0.0980450)
    kwargs.setdefault("maxSeedsPerSpM" , 5)
    kwargs.setdefault("deltaRMiddleMinSPRange" , 0 * ActsUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange" , 0 * ActsUnits.mm)
    kwargs.setdefault("zBinEdges", [ -2800. , -2500. , -1400. , -925. , -450. , -250. , 250. , 450. , 925. , 1400. , 2500. , 2800.])
    kwargs.setdefault("zBinsCustomLooping" , [6, 7, 8, 9, 10, 11, 5, 4, 3, 2, 1])
    kwargs.setdefault("zBinNeighborsTop", [
        (0, 0), (-1, 0), (-1, 0), (-1, 0), (-1, 0), (-2, 2),
        (0, 1), (0, 1), (0, 1), (0, 1), (0, 0)
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
        (0, 0), (0, 1), (0, 1), (0, 1), (0, 1), (0, 0),
        (-1, 0), (-1, 0), (-1, 0), (-1, 0), (0, 0)
    ])
    kwargs.setdefault("doSeedQualitySelection", True)

    # Seed confirmation
    kwargs.setdefault("seedConfCentralZMin", -450.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralZMax", 450.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralRMax", 140.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralNTopLargeR", 0)
    kwargs.setdefault("seedConfCentralNTopSmallR", 0)
    kwargs.setdefault("seedConfCentralMinBottomRadius", 40.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMaxZOrigin", 200.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMinImpact", 1.0 * ActsUnits.mm)

    kwargs.setdefault("seedConfForwardZMin", -2800.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMax", 2800.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardRMax", 140.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardNTopLargeR", 0)
    kwargs.setdefault("seedConfForwardNTopSmallR", 0)
    kwargs.setdefault("seedConfForwardMinBottomRadius", 40.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMaxZOrigin", 200.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMinImpact", 1.0 * ActsUnits.mm)

    kwargs.setdefault("seedConfirmation" , True) 
    kwargs.setdefault("seedConfirmationInFilter", False) 

    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name, **kwargs))
    return acc

def ActsInDetStripSeedingToolCfg(flags,
                            name: str = "ActsInDetStripSeedingTool",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## ACTS seeding tool configurations for Run 3 InnerDetector SCT

    impactMax = 20. * ActsUnits.mm
    collisionRegionAbsMax = 200. * ActsUnits.mm 

    kwargs.setdefault("doSeedQualitySelection", False)
    # For SpacePointGridConfig
    kwargs.setdefault("gridRMax" , 600. * ActsUnits.mm)
    kwargs.setdefault("deltaRMin" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMax" , 600. * ActsUnits.mm) 
    kwargs.setdefault("impactMax" , impactMax)
    # For SeedfinderConfig
    kwargs.setdefault("rMax" , flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("deltaRMinTopSP" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 300. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 10. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 300. * ActsUnits.mm) 
    kwargs.setdefault("deltaZMax" , 900. * ActsUnits.mm)
    kwargs.setdefault("interactionPointCut" , False) 

    kwargs.setdefault("deltaRMiddleMinSPRange" , 5 * ActsUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange" , 5 * ActsUnits.mm)
    kwargs.setdefault("useDetailedDoubleMeasurementInfo" , True)
    kwargs.setdefault("maxPtScattering", float("inf"))
    # For SeedFilterConfig
    kwargs.setdefault("useDeltaRorTopRadius" , False)
    kwargs.setdefault("seedConfirmationInFilter" , False)
    kwargs.setdefault("impactWeightFactor" , 1.)
    kwargs.setdefault("compatSeedLimit" , 4)
    kwargs.setdefault("numSeedIncrement" , 1.)
    kwargs.setdefault("seedWeightIncrement" , 10100.) 
    kwargs.setdefault("maxSeedsPerSpMConf" , 100) 
    kwargs.setdefault("maxQualitySeedsPerSpMConf" , 100) 
    # For seeding algorithm
    kwargs.setdefault("zBinNeighborsTop", [
        (0, 0), (-1, 0), (-1, 0), (-1, 0), (-1, 0), (-1, 1),
        (0, 1), (0, 1), (0, 1), (0, 1), (0, 0)
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
        (0, 0), (0, 1), (0, 1), (0, 2), (0, 1), (0, 0),
        (-1, 0), (-2, 0), (-1, 0), (-1, 0), (0, 0)
    ])
    kwargs.setdefault("zBinsCustomLooping" , [6, 7, 8, 9, 10, 11, 5, 4, 3, 2, 1])
    kwargs.setdefault("zBinEdges", [ -2800. , -2500. , -1400. , -925. , -450. , -250. , 250. , 450. , 925. , 1400. , 2500. , 2800.])

    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("collisionRegionMin", -1. * collisionRegionAbsMax)
    kwargs.setdefault("collisionRegionMax", collisionRegionAbsMax)

    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT / GaudiUnits.GeV * ActsUnits.GeV)
    kwargs.setdefault("cotThetaMax" , 7.40626311) # eta = 2.7
    kwargs.setdefault("zMax", 2800 * ActsUnits.mm)
    kwargs.setdefault("zMin", -2800 * ActsUnits.mm)
    
    kwargs.setdefault("phiBinDeflectionCoverage" , 1)
    kwargs.setdefault("maxPhiBins" , 200)
    kwargs.setdefault("sigmaScattering", 5.0)
    kwargs.setdefault("maxSeedsPerSpM" , 5)

    # Seed confirmation
    kwargs.setdefault("seedConfirmation" , True) 
    kwargs.setdefault("seedConfCentralZMin", -450.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralZMax", 450.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralRMax", 140.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralNTopLargeR", 1)
    kwargs.setdefault("seedConfCentralNTopSmallR", 2)
    kwargs.setdefault("seedConfCentralMinBottomRadius", 40.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMaxZOrigin", 200.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMinImpact", 1.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMin", -2800.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMax", 2800.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardRMax", 140.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardNTopLargeR", 1)
    kwargs.setdefault("seedConfForwardNTopSmallR", 2)
    kwargs.setdefault("seedConfForwardMinBottomRadius", 40.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMaxZOrigin", 200.0 * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMinImpact", 1.0 * ActsUnits.mm)

    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name, **kwargs))
    return acc

# ACTS algorithm using Athena objects upstream
def ActsInDetPixelSeedingAlgCfg(flags,
                           name: str = 'ActsInDetPixelSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc.merge(PixelReadoutGeometryCfg(flags))

    useFastTracking = False

    if "SeedTool" not in kwargs:
        kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsInDetPixelSeedingToolCfg(flags)))

    kwargs.setdefault("useFastTracking", useFastTracking)
    kwargs.setdefault('InputSpacePoints', ['PixelSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ActsPixelSeeds')
    kwargs.setdefault('UsePixel', True)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        pass

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsInDetStripSeedingAlgCfg(flags,
                           name: str = 'ActsInDetStripSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

    if "SeedTool" not in kwargs:
        kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsInDetStripSeedingToolCfg(flags)))

    kwargs.setdefault('InputSpacePoints', ['SCT_SpacePoints', 'OverlapSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ActsSCT_Seeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        pass

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsInDetMainSeedingCfg(flags,
                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('processPixels', flags.Detector.EnablePixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableSCT)
    kwargs.setdefault('estimateParameters', flags.Acts.Seeds.doAnalysis)

    if kwargs['processPixels']:
        acc.merge(ActsInDetPixelSeedingAlgCfg(flags, **extractChildKwargs(prefix='PixelSeedingAlg.', **kwargs)))
    if kwargs['processStrips']:
        acc.merge(ActsInDetStripSeedingAlgCfg(flags, **extractChildKwargs(prefix='StripSeedingAlg.', **kwargs)))

    
    if kwargs['estimateParameters']:
        if kwargs['processPixels']:
            from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg
            acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(flags,
                                                        **extractChildKwargs(prefix='PixelSeedsToTrackParamsAlg.', **kwargs)))

        if kwargs['processStrips']:
            from ActsConfig.ActsAnalysisConfig import ActsStripSeedsToTrackParamsAlgCfg
            acc.merge(ActsStripSeedsToTrackParamsAlgCfg(flags,
                                                        **extractChildKwargs(prefix='StripSeedsToTrackParamsAlg.', **kwargs)))
            
    return acc

def ActsInDetSeedingCfg(flags,**kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    processPixels = flags.Detector.EnablePixel
    processStrips = flags.Detector.EnableSCT

    kwargs.setdefault('processPixels', processPixels)
    kwargs.setdefault('processStrips', processStrips)
    kwargs.setdefault('estimateParameters', flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis)

    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    if processPixels:
        # Seeding algo
        kwargs.setdefault('PixelSeedingAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSeedingAlg')
        kwargs.setdefault('PixelSeedingAlg.useFastTracking', isFastPrimaryPass(flags))
        kwargs.setdefault('PixelSeedingAlg.OutputSeeds', f'{flags.Tracking.ActiveConfig.extension}PixelSeeds')

        pixelSpacePoints = ['PixelSpacePoints']
        kwargs.setdefault('PixelSeedingAlg.InputSpacePoints', pixelSpacePoints)

        # Setup the seed to track parameters algorithms either if we persistify them or we want to run the ActsMonitoring
        if flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSeedsToTrackParamsAlg')
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.InputSeedContainerKey', kwargs['PixelSeedingAlg.OutputSeeds'])
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey', f'{flags.Tracking.ActiveConfig.extension}PixelEstimatedTrackParams')
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.DetectorElementsKey' , 'PixelDetectorElementCollection')

    if processStrips:
        # Seeding algo
        kwargs.setdefault('StripSeedingAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSeedingAlg')
        kwargs.setdefault('StripSeedingAlg.OutputSeeds', f'{flags.Tracking.ActiveConfig.extension}SCT_Seeds')
        kwargs.setdefault('StripSeedingAlg.InputSpacePoints', ['SCT_SpacePoints', 'OverlapSpacePoints'])
            
        if flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('StripSeedsToTrackParamsAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSeedsToTrackParamsAlg')
            kwargs.setdefault('StripSeedsToTrackParamsAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripSeedsToTrackParamsAlg.InputSeedContainerKey', kwargs['StripSeedingAlg.OutputSeeds'])
            kwargs.setdefault('StripSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey', f'{flags.Tracking.ActiveConfig.extension}SCT_EstimatedTrackParams')
            kwargs.setdefault('StripSeedsToTrackParamsAlg.DetectorElementsKey' , 'SCT_DetectorElementCollection')
            
    acc.merge(ActsInDetMainSeedingCfg(flags, **kwargs))        

    if flags.Tracking.ActiveConfig.storeTrackSeeds:
        acc.merge(ActsInDetStoreTrackSeedsCfg(flags,
                                         processPixels = processPixels,
                                         processStrips = processStrips))

    return acc

def ActsInDetStoreTrackSeedsCfg(flags,
                           *,
                           processPixels: bool,
                           processStrips: bool,
                           **kwargs: dict) -> ComponentAccumulator:


    acc = ComponentAccumulator()
    
    seedKeyPixels = f'{flags.Tracking.ActiveConfig.extension}PixelSeeds'
    seedKeyStrips = f'{flags.Tracking.ActiveConfig.extension}SCT_Seeds'
    paramsKeyPixels = f'{flags.Tracking.ActiveConfig.extension}PixelEstimatedTrackParams'
    paramsKeyStrips = f'{flags.Tracking.ActiveConfig.extension}SCT_EstimatedTrackParams'
    trackKeyPixels = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}PixelTracks'
    trackKeyStrips = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}StripTracks'
    particleKeyPixels = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}PixelTrackParticles'
    particleKeyStrips = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}StripTrackParticles'

    trackKey = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}Tracks'
    particleKey = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}TrackParticles'

    from ActsConfig.ActsSeedingConfig import ActsSeedToTrackCnvAlgCfg

    if processPixels:
        # Create track parameters from pixel seeds
        from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg
        acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(flags,
                                                    name = f'{flags.Tracking.ActiveConfig.extension}PixelSeedsToTrackParamsAlg',
                                                    extension = flags.Tracking.ActiveConfig.extension,
                                                    InputSeedContainerKey = seedKeyPixels,
                                                    OutputTrackParamsCollectionKey = paramsKeyPixels,
                                                    DetectorElementsKey = 'PixelDetectorElementCollection'))

        # Convert pixel seed to Acts track
        acc.merge(ActsSeedToTrackCnvAlgCfg(flags,
                                           name=f"{flags.Tracking.ActiveConfig.extension}PixelSeedToTrackCnvAlg",
                                           EstimatedTrackParametersKey = [paramsKeyPixels],
                                           SeedContainerKey = [seedKeyPixels],
                                           ACTSTracksLocation = trackKeyPixels))

        # Truth
        if flags.Tracking.doTruth:
            from ActsConfig.ActsTruthConfig import ActsTrackFindingValidationAlgCfg, ActsInDetTrackToTruthAssociationAlgCfg
            acc.merge(ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{trackKeyPixels}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = trackKeyPixels,
                                                        AssociationMapOut = f"{trackKeyPixels}ToTruthParticleAssociation"))

            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{trackKeyPixels}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{trackKeyPixels}ToTruthParticleAssociation"))
            
        # Track Particle creation and persistification
        # - input track collection: trackKeyPixels
        # - output track particle collection: particleKeyPixels
        # although the name has 'ITk', it seems okay for Inner Detector
        from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
        acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                  TrackContainers = [trackKeyPixels],
                                                  TrackParticleContainer = particleKeyPixels))

                    
    if processStrips:
        # Create track parameters from strip seeds 
        from ActsConfig.ActsAnalysisConfig import ActsStripSeedsToTrackParamsAlgCfg
        acc.merge(ActsStripSeedsToTrackParamsAlgCfg(flags,
                                                    name = f'{flags.Tracking.ActiveConfig.extension}StripSeedsToTrackParamsAlg',
                                                    extension = flags.Tracking.ActiveConfig.extension,
                                                    InputSeedContainerKey = seedKeyStrips,
                                                    OutputTrackParamsCollectionKey = paramsKeyStrips,
                                                    DetectorElementsKey = 'SCT_DetectorElementCollection'))

        # Convert strip seed to Acts track
        acc.merge(ActsSeedToTrackCnvAlgCfg(flags, 
                                           name=f"{flags.Tracking.ActiveConfig.extension}StripSeedToTrackCnvAlg",
                                           EstimatedTrackParametersKey = [paramsKeyStrips],
                                           SeedContainerKey = [seedKeyStrips],
                                           ACTSTracksLocation = trackKeyStrips))

        # Truth
        if flags.Tracking.doTruth:
            from ActsConfig.ActsTruthConfig import ActsTrackFindingValidationAlgCfg, ActsInDetTrackToTruthAssociationAlgCfg
            acc.merge(ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                                        name=f"{trackKeyStrips}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = trackKeyStrips,
                                                        AssociationMapOut = f"{trackKeyStrips}ToTruthParticleAssociation"))

            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{trackKeyStrips}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{trackKeyStrips}ToTruthParticleAssociation"))

        # Track Particle creation and persistification
        # - input track collection: trackKeyStrips
        # - output track particle collection: particleKeyStrips
        from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
        acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                  TrackContainers = [trackKeyStrips],
                                                  TrackParticleContainer = particleKeyStrips))

    # If both pixel and strips are processed, also make track particles from the sum
    # This will provide the complete seed efficiency for ACTS
    if processPixels and processStrips:
      # Parameter estimation has already been performed
      # Convert seeds to Acts tracks
      acc.merge(ActsSeedToTrackCnvAlgCfg(flags,
                                         name=f"{flags.Tracking.ActiveConfig.extension}SeedToTrackCnvAlg",
                                         EstimatedTrackParametersKey = [paramsKeyPixels, paramsKeyStrips],
                                         SeedContainerKey = [seedKeyPixels, seedKeyStrips],
                                         ACTSTracksLocation = trackKey))
      
      # Truth
      if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTrackFindingValidationAlgCfg, ActsInDetTrackToTruthAssociationAlgCfg
        acc.merge(ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                                    name=f"{trackKey}TrackToTruthAssociationAlg",
                                                    ACTSTracksLocation = trackKey,
                                                    AssociationMapOut = f"{trackKey}ToTruthParticleAssociation"))
        
        acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                   name = f"{trackKey}TrackFindingValidationAlg",
                                                   TrackToTruthAssociationMap = f"{trackKey}ToTruthParticleAssociation"))
        
      # Track Particle creation and persistification
      # - input track collection: trackKey
      # - output track particle collection: particleKey
      from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
      acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                TrackContainers = [trackKey],
                                                TrackParticleContainer = particleKey))
      
    return acc
