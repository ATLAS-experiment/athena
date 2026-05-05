# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SeedingStrategy
from ActsConfig.ActsUtilities import extractChildKwargs
from AthenaCommon.Utils.unixtools import find_datafile
from ActsInterop import UnitConstants as ActsUnits
import AthenaCommon.SystemOfUnits as GaudiUnits

def ActsGbtsFtfSeedingTrigToolCfg(flags,name: str = "GbtsFtfActsSeedingTool", **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  if "layerNumberTool" not in kwargs:
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import ITkTrigL2LayerNumberToolCfg
    ntargs = {"UseNewLayerScheme" : True}
    kwargs.setdefault("layerNumberTool",acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags,**ntargs)))
  
  kwargs.setdefault("DoPhiFiltering", False) #no phi-filtering for full-scan tracking
  kwargs.setdefault("UseBeamTilt", False)

  isLargeD0 = flags.Tracking.ActiveConfig.extension in ["LargeD0", "ActsLargeRadius"]
  
  kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPTSeed)
  kwargs.setdefault("MaxGraphEdges", 3000000)
  kwargs.setdefault("ConnectionFileName",
                    "binTables_ITK_RUN4_LRT.txt" if isLargeD0 else "binTables_ITK_RUN4.txt")

  acc.setPrivateTools(CompFactory.GbtsFtfActsSeedingTool(name, **kwargs))

  return acc


# ACTS tools
def ActsPixelSeedingToolCfg(flags,
                            name: str = "ActsPixelSeedingTool",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    ## For ITkPixel
    kwargs.setdefault("numSeedIncrement" , float("inf"))
    kwargs.setdefault("deltaZMax" , float("inf"))
    kwargs.setdefault("maxPtScattering", float("inf"))
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("rMax", 320. * ActsUnits.mm)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed / GaudiUnits.GeV * ActsUnits.GeV)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed / GaudiUnits.mm * ActsUnits.mm)
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("rRangeMiddleSP", [
        [0,0],
        [140, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [70, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [40, 260],
        [140, 260],
        [0, 0]])
    acc.setPrivateTools(CompFactory.ActsTrk.GridTripletSeedingTool(name, **kwargs))
    return acc

def ActsFastPixelSeedingToolCfg(flags,
                                name: str = "ActsFastPixelSeedingTool",
                                **kwargs) -> ComponentAccumulator:
    ## Additional cuts for fast seed configuration
    kwargs.setdefault("sigmaScattering", 2.)
    kwargs.setdefault("maxSeedsPerSpM", 3)
    kwargs.setdefault("collisionRegionMin", -150 * ActsUnits.mm)
    kwargs.setdefault("collisionRegionMax", 150 * ActsUnits.mm)
    kwargs.setdefault("maxPhiBins", 200)
    kwargs.setdefault("gridRMax", 250 * ActsUnits.mm)
    kwargs.setdefault("deltaRMax", 200 * ActsUnits.mm)
    kwargs.setdefault("zBinsCustomLooping" , [2, 10, 3, 9, 6, 4, 8, 5, 7])
    kwargs.setdefault("rRangeMiddleSP", [
             [0.0, 0.0],
             [60.0, 165.0],
             [60.0, 200.0],
             [60.0, 200.0],
             [60.0, 260.0],
             [60.0, 260.0],
             [60.0, 260.0],
             [60.0, 200.0],
             [60.0, 200.0],
             [60.0, 165.0],
             [0.0, 0.0]])

    kwargs.setdefault("zBinNeighborsTop", [
      [0, 0], # -3000, -2000  
      [-1, 0], # -2000, -1400  
      [-1, 0], # -1400, -910
      [-1, 0], # -910, -500 
      [-1, 0], # -500, -250 
      [-1, 1], # -250, 250
      [0, 1], # 250, 500 
      [0, 1], # 500, 910
      [0, 1], # 910, 1400
      [0, 1], # 1400, 2000
      [0, 0] # 2000, 3000
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
      [0, 0], # -3000, -2000
      [0, 1], # -2000, -1400      
      [0, 1], # -1400, -910
      [0, 1], # -910, -500
      [0, 1], # -500, -250
      [0, 0], # -250, 250
      [-1, 0], # 250, 500
      [-1, 0], # 500, 910
      [-1, 0], # 910, 1400
      [-1, 0], # 1400, 2000
      [0, 0] # 2000, 3000
    ])
    
    kwargs.setdefault("zBinEdges", [-3000., -2000, -1400., -910., -500., -250.,  250., 500., 910., 1400., 2000, 3000.])
    kwargs.setdefault("useExperimentCuts", True)

    kwargs.setdefault("deltaRMaxTopSP", 220 * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 135 * ActsUnits.mm)

    return ActsPixelSeedingToolCfg(flags, name, **kwargs)

def ActsStripSeedingToolCfg(flags,
                            name: str = "ActsStripSeedingTool",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("doSeedQualitySelection", False)
    # For SpacePointGridConfig
    kwargs.setdefault("gridRMax", 1000. * ActsUnits.mm)
    kwargs.setdefault("deltaRMax", 600. * ActsUnits.mm)
    kwargs.setdefault("impactMax", 20. * ActsUnits.mm)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed / GaudiUnits.GeV * ActsUnits.GeV)
    # For SeedfinderConfig
    kwargs.setdefault("rMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("deltaRMinTopSP", 20. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP", 300. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP", 20. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 300. * ActsUnits.mm)
    kwargs.setdefault("deltaZMax", 900. * ActsUnits.mm)
    kwargs.setdefault("interactionPointCut", False)
    kwargs.setdefault("zBinsCustomLooping", [7, 8, 6, 9, 5, 10, 4, 11, 3, 12, 2])
    kwargs.setdefault("deltaRMiddleMinSPRange", 30 * ActsUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange", 150 * ActsUnits.mm)
    kwargs.setdefault("useDetailedDoubleMeasurementInfo", True)
    kwargs.setdefault("maxPtScattering", float("inf"))
    # For SeedFilterConfig
    kwargs.setdefault("useDeltaRorTopRadius", False)
    kwargs.setdefault("seedConfirmationInFilter", False)
    kwargs.setdefault("impactWeightFactor", 1.)
    kwargs.setdefault("compatSeedLimit", 4)
    kwargs.setdefault("numSeedIncrement", 1.)
    kwargs.setdefault("seedWeightIncrement", 10100.)
    kwargs.setdefault("maxSeedsPerSpMConf", 100)
    kwargs.setdefault("maxQualitySeedsPerSpMConf", 100)
    # For seeding algorithm
    kwargs.setdefault("zBinNeighborsBottom", [(0,0),(0,1),(0,1),(0,1),(0,2),(0,1),(0,0),(-1,0),(-2,0),(-1,0),(-1,0),(-1,0),(0,0)])
    # Any other
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
    kwargs.setdefault("collisionRegionMin", -200. * ActsUnits.mm)
    kwargs.setdefault("collisionRegionMax", 200. * ActsUnits.mm)

    acc.setPrivateTools(CompFactory.ActsTrk.GridTripletSeedingTool(name, **kwargs))
    return acc

def ActsLargeRadiusStripSeedingToolCfg(flags,
                                        name: str = "ActsLargeRadiusStripSeedingTool",
                                        **kwargs) -> ComponentAccumulator:
    ## LRT-specific seeding cuts
    kwargs.setdefault("interactionPointCut", True)
    kwargs.setdefault("impactMax", 300. * ActsUnits.mm)
    kwargs.setdefault("collisionRegionMin", -500. * ActsUnits.mm)
    kwargs.setdefault("collisionRegionMax", 500. * ActsUnits.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange", 75 * ActsUnits.mm)
    kwargs.setdefault("deltaRMinTopSP", 50. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxTopSP", 250. * ActsUnits.mm)
    kwargs.setdefault("deltaRMinBottomSP", 50. * ActsUnits.mm)
    kwargs.setdefault("deltaRMaxBottomSP", 250. * ActsUnits.mm)
    kwargs.setdefault("deltaZMax", 850. * ActsUnits.mm)
    kwargs.setdefault("cotThetaMax", 5.0)
    kwargs.setdefault("maxSeedsPerSpM", 1)
    kwargs.setdefault("maxStripDeltaCotTheta", 0.3)
    kwargs.setdefault("absDeltaEtaWeightFactor", 10.)
    kwargs.setdefault("absDeltaEtaMinImpact", 2.)
    kwargs.setdefault("zBinEdges", [-3000., -2500, -1400., -910., -500., -250.,  250., 500., 910., 1400., 2500, 3000.])
    kwargs.setdefault("zBinsCustomLooping" , [2, 10, 3, 9, 6, 4, 8, 5, 7])
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("zBinNeighborsTop", [
      [0, 0],  # -3000, -2500
      [-1, 0], # -2500, -1400
      [-1, 0], # -1400, -910
      [-1, 0], # -910, -500
      [-1, 0], # -500, -250
      [-1, 1], # -250, 250
      [0, 1],  # 250, 500
      [0, 1],  # 500, 910
      [0, 1],  # 910, 1400
      [0, 1],  # 1400, 2500
      [0, 0]   # 2500, 3000
    ])
    kwargs.setdefault("zBinNeighborsBottom", [
      [0, 0],  # -3000, -2500
      [0, 1],  # -2500, -1400
      [0, 1],  # -1400, -910
      [0, 1],  # -910, -500
      [0, 1],  # -500, -250
      [0, 0],  # -250, 250
      [-1, 0], # 250, 500
      [-1, 0], # 500, 910
      [-1, 0], # 910, 1400
      [-1, 0], # 1400, 2500
      [0, 0]   # 2500, 3000
    ])
    kwargs.setdefault("rRangeMiddleSP", [
      [0.0, 0.0],       # -3000, -2500
      [400.0, 850.0],   # -2500, -1400
      [500.0, 800.0],   # -1400, -910
      [500.0, 800.0],   # -910, -500
      [500.0, 800.0],   # -500, -250
      [500.0, 800.0],   # -250, 250
      [500.0, 800.0],   # 250, 500
      [500.0, 800.0],   # 500, 910
      [500.0, 800.0],   # 910, 1400
      [400.0, 850.0],   # 1400, 2500
      [0.0, 0.0]        # 2500, 3000
    ])

    kwargs.setdefault("seedConfirmation", True)
    kwargs.setdefault("seedConfirmationInFilter", True)
    kwargs.setdefault("zOriginWeightFactor", 1.)
    kwargs.setdefault("maxSeedsPerSpMConf", 1)
    kwargs.setdefault("maxQualitySeedsPerSpMConf", 1)
    kwargs.setdefault("seedConfCentralZMin",              -1400. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralZMax",               1400. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralRMax",                140. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralNTopLargeR",         1)
    kwargs.setdefault("seedConfCentralNTopSmallR",         0)
    kwargs.setdefault("seedConfCentralMinBottomRadius",      0. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMaxZOrigin",        1500. * ActsUnits.mm)
    kwargs.setdefault("seedConfCentralMinImpact",          200. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMin",              -3000. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardZMax",               3000. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardRMax",                140. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardNTopLargeR",         1)
    kwargs.setdefault("seedConfForwardNTopSmallR",         0)
    kwargs.setdefault("seedConfForwardMinBottomRadius",    350. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMaxZOrigin",        1500. * ActsUnits.mm)
    kwargs.setdefault("seedConfForwardMinImpact",          200. * ActsUnits.mm)

    return ActsStripSeedingToolCfg(flags, name, **kwargs)

def ActsPixelGbtsSeedingToolCfg(flags,
                                name: str = "ActsPixelGbtsSeedingTool", 
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if "layerNumberTool" not in kwargs:
        from TrigFastTrackFinder.TrigFastTrackFinderConfig import ITkTrigL2LayerNumberToolCfg
        ntargs = {"UseNewLayerScheme": True}
        kwargs.setdefault(
            "layerNumberTool",
            acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags, **ntargs))
        )
    ## For ITkPixel, use default values for ActsTrk::GbtsSeedingTool
    kwargs.setdefault("connectorInputFile" , find_datafile("binTables_ITK_RUN4.txt"))
    kwargs.setdefault("lutInputFile" , find_datafile("gbts_ml_pixel_barrel_loose.lut"))
    kwargs.setdefault("minPt" , flags.Tracking.ActiveConfig.minPTSeed / GaudiUnits.GeV * ActsUnits.GeV)

    acc.setPrivateTools(CompFactory.ActsTrk.GbtsSeedingTool(name = name, **kwargs))
    return acc

# ACTS algorithm using Athena objects upstream
def ActsPixelSeedingAlgCfg(flags,
                           name: str = 'ActsPixelSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    useFastTracking = kwargs.get("useFastTracking", isFastPrimaryPass(flags))

    if "SeedTool" not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Gbts:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsPixelGbtsSeedingToolCfg(flags)))
        elif flags.Acts.SeedingStrategy is SeedingStrategy.GbtsFtf:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsGbtsFtfSeedingTrigToolCfg(flags)))
        else:
            if useFastTracking:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsFastPixelSeedingToolCfg(flags)))
            else:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsPixelSeedingToolCfg(flags)))

    kwargs.setdefault("useFastTracking", useFastTracking)
    kwargs.setdefault('InputSpacePoints', ['ITkPixelSpacePoints_Cached'] if flags.Acts.useCache else ['ITkPixelSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ActsPixelSeeds')
    kwargs.setdefault('UsePixel', True)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkPixelSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkPixelSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.GenericSeedingAlg(name, **kwargs))
    return acc


def ActsStripSeedingAlgCfg(flags,
                           name: str = 'ActsStripSeedingAlg',
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    if "SeedTool" not in kwargs:
        if flags.Tracking.ActiveConfig.isLargeD0:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsLargeRadiusStripSeedingToolCfg(flags)))
        else:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsStripSeedingToolCfg(flags)))

    kwargs.setdefault('InputSpacePoints', ['ITkStripSpacePoints_Cached', 'ITkStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ActsStripSeeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkStripSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.GenericSeedingAlg(name, **kwargs))
    return acc


def ActsMainSeedingCfg(flags,
                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('processPixels', flags.Detector.EnableITkPixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableITkStrip)
    kwargs.setdefault('estimateParameters', flags.Acts.Seeds.doAnalysis)

    if kwargs['processPixels']:
        acc.merge(ActsPixelSeedingAlgCfg(flags, **extractChildKwargs(prefix='PixelSeedingAlg.', **kwargs)))
    if kwargs['processStrips']:
        acc.merge(ActsStripSeedingAlgCfg(flags, **extractChildKwargs(prefix='StripSeedingAlg.', **kwargs)))

    
    if kwargs['estimateParameters']:
        if kwargs['processPixels']:
            from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg
            acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(flags,
                                                        **extractChildKwargs(prefix='PixelSeedsToTrackParamsAlg.', **kwargs)))

        if kwargs['processStrips']:
            from ActsConfig.ActsAnalysisConfig import ActsStripSeedsToTrackParamsAlgCfg
            acc.merge(ActsStripSeedsToTrackParamsAlgCfg(flags,
                                                        **extractChildKwargs(prefix='StripSeedsToTrackParamsAlg.', **kwargs)))

    if flags.Acts.Seeds.doAnalysis:
        if kwargs['processPixels']:
            from ActsConfig.ActsAnalysisConfig import ActsPixelSeedAnalysisAlgCfg, ActsPixelEstimatedTrackParamsAnalysisAlgCfg
            acc.merge(ActsPixelSeedAnalysisAlgCfg(flags, **extractChildKwargs(prefix='PixelSeedAnalysisAlg.', **kwargs)))
            acc.merge(ActsPixelEstimatedTrackParamsAnalysisAlgCfg(flags, **extractChildKwargs(prefix='PixelEstimatedTrackParamsAnalysisAlg.', **kwargs)))
            
        if kwargs['processStrips']:
            from ActsConfig.ActsAnalysisConfig import ActsStripSeedAnalysisAlgCfg, ActsStripEstimatedTrackParamsAnalysisAlgCfg
            acc.merge(ActsStripSeedAnalysisAlgCfg(flags, **extractChildKwargs(prefix='StripSeedAnalysisAlg.', **kwargs)))
            acc.merge(ActsStripEstimatedTrackParamsAnalysisAlgCfg(flags, **extractChildKwargs(prefix='StripEstimatedTrackParamsAnalysisAlg.', **kwargs)))
            
    return acc

def ActsSeedingCfg(flags,**kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    processPixels = flags.Detector.EnableITkPixel
    processStrips = flags.Detector.EnableITkStrip

    # For conversion pass we do not process pixels
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    if flags.Tracking.ActiveConfig.extension in ["ActsConversion", "ActsLargeRadius", "ActsValidateLargeRadiusStandalone"]:
        processPixels = False
    # For main pass disable strips if fast tracking configuration
    elif isFastPrimaryPass(flags):
        processStrips = False

    kwargs.setdefault('processPixels', processPixels)
    kwargs.setdefault('processStrips', processStrips)
    kwargs.setdefault('estimateParameters', flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis)

    # TO-DO: refactor this seeding tool configuration
    if flags.Tracking.ActiveConfig.extension == "ActsHeavyIon" and processPixels:
        kwargs.setdefault('PixelSeedingAlg.SeedTool', acc.popToolsAndMerge(ActsPixelSeedingToolCfg(flags,
                                                                                                   name=f'{flags.Tracking.ActiveConfig.extension}PixelSeedingTool')))

    if processStrips and flags.Acts.SeedingStrategy is SeedingStrategy.GridTriplet:
        if flags.Tracking.ActiveConfig.isLargeD0:
            kwargs.setdefault('StripSeedingAlg.SeedTool', acc.popToolsAndMerge(ActsLargeRadiusStripSeedingToolCfg(flags,
                                                                                                                   name=f'{flags.Tracking.ActiveConfig.extension}StripSeedingTool')))
        else:
            kwargs.setdefault('StripSeedingAlg.SeedTool', acc.popToolsAndMerge(ActsStripSeedingToolCfg(flags,
                                                                                                       name=f'{flags.Tracking.ActiveConfig.extension}StripSeedingTool')))
        
    if processPixels:
        # Seeding algo
        kwargs.setdefault('PixelSeedingAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSeedingAlg')
        kwargs.setdefault('PixelSeedingAlg.useFastTracking', isFastPrimaryPass(flags))
        kwargs.setdefault('PixelSeedingAlg.OutputSeeds', f'{flags.Tracking.ActiveConfig.extension}PixelSeeds')

        pixelSpacePoints = ['ITkPixelSpacePoints_Cached'] if flags.Acts.useCache else ['ITkPixelSpacePoints']        
        if flags.Tracking.ActiveConfig.isSecondaryPass:
            pixelSpacePoints = [f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}PixelSpacePoints_Cached'] if flags.Acts.useCache else [f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}PixelSpacePoints']
        kwargs.setdefault('PixelSeedingAlg.InputSpacePoints', pixelSpacePoints)

        # Setup the seed to track parameters algorithms either if we persistify them or we want to run the ActsMonitoring
        if flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSeedsToTrackParamsAlg')
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.InputSeedContainerKey', kwargs['PixelSeedingAlg.OutputSeeds'])
            kwargs.setdefault('PixelSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey', f'{flags.Tracking.ActiveConfig.extension}PixelEstimatedTrackParams')
                    
        # Analysis algo(s)
        if flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('PixelSeedAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSeedAnalysisAlg')
            kwargs.setdefault('PixelSeedAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelSeedAnalysisAlg.InputSeedCollection', kwargs['PixelSeedingAlg.OutputSeeds'])

            kwargs.setdefault('PixelEstimatedTrackParamsAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelEstimatedTrackParamsAnalysisAlg')
            kwargs.setdefault('PixelEstimatedTrackParamsAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelEstimatedTrackParamsAnalysisAlg.InputTrackParamsCollection', kwargs['PixelSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey'])

    if processStrips:
        # Seeding algo
        kwargs.setdefault('StripSeedingAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSeedingAlg')
        kwargs.setdefault('StripSeedingAlg.OutputSeeds', f'{flags.Tracking.ActiveConfig.extension}StripSeeds')
        # Conversion pass does not use overlap space points
        # Space Point naming is not yet fully connected to tracking passes - this will change
        if flags.Tracking.ActiveConfig.extension == 'ActsConversion':
            kwargs.setdefault('StripSeedingAlg.InputSpacePoints', ['ITkConversionStripSpacePoints_Cached'] if flags.Acts.useCache else ['ITkConversionStripSpacePoints'])
        elif flags.Tracking.ActiveConfig.extension == 'ActsLargeRadius':
            kwargs.setdefault('StripSeedingAlg.InputSpacePoints', ['ITkLargeRadiusStripSpacePoints_Cached',
                                                                   'ITkLargeRadiusStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkLargeRadiusStripSpacePoints',
                                                                                                                                                'ITkLargeRadiusStripOverlapSpacePoints'])
        elif flags.Tracking.ActiveConfig.extension == 'ActsLowPt':
            kwargs.setdefault('StripSeedingAlg.InputSpacePoints', ['ITkLowPtStripSpacePoints_Cached',
                                                                   'ITkLowPtStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkLowPtStripSpacePoints',
                                                                                                                                          'ITkLowPtStripOverlapSpacePoints'])
        else:
            kwargs.setdefault('StripSeedingAlg.InputSpacePoints', ['ITkStripSpacePoints_Cached',
                                                                   'ITkStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkStripSpacePoints',
                                                                                                                                     'ITkStripOverlapSpacePoints'])
            
        if flags.Tracking.ActiveConfig.storeTrackSeeds or flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('StripSeedsToTrackParamsAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSeedsToTrackParamsAlg')
            kwargs.setdefault('StripSeedsToTrackParamsAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripSeedsToTrackParamsAlg.InputSeedContainerKey', kwargs['StripSeedingAlg.OutputSeeds'])
            kwargs.setdefault('StripSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey', f'{flags.Tracking.ActiveConfig.extension}StripEstimatedTrackParams')
            
            
        # Analysis algo(s)
        if flags.Acts.Seeds.doAnalysis:
            kwargs.setdefault('StripSeedAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSeedAnalysisAlg')
            kwargs.setdefault('StripSeedAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripSeedAnalysisAlg.InputSeedCollection', kwargs['StripSeedingAlg.OutputSeeds'])

            kwargs.setdefault('StripEstimatedTrackParamsAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripEstimatedTrackParamsAnalysisAlg')
            kwargs.setdefault('StripEstimatedTrackParamsAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripEstimatedTrackParamsAnalysisAlg.InputTrackParamsCollection', kwargs['StripSeedsToTrackParamsAlg.OutputTrackParamsCollectionKey'])
            
    acc.merge(ActsMainSeedingCfg(flags, **kwargs))        


    if flags.Tracking.ActiveConfig.storeTrackSeeds:
        acc.merge(ActsStoreTrackSeedsCfg(flags,
                                         processPixels = processPixels,
                                         processStrips = processStrips))

    return acc

def ActsStoreTrackSeedsCfg(flags,
                           *,
                           processPixels: bool,
                           processStrips: bool,
                           **kwargs: dict) -> ComponentAccumulator:


    acc = ComponentAccumulator()
    
    seedKeyPixels = f'{flags.Tracking.ActiveConfig.extension}PixelSeeds'
    seedKeyStrips = f'{flags.Tracking.ActiveConfig.extension}StripSeeds'
    paramsKeyPixels = f'{flags.Tracking.ActiveConfig.extension}PixelEstimatedTrackParams'
    paramsKeyStrips = f'{flags.Tracking.ActiveConfig.extension}StripEstimatedTrackParams'
    trackKeyPixels = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}PixelTracks'
    trackKeyStrips = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}StripTracks'
    particleKeyPixels = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}PixelTrackParticles'
    particleKeyStrips = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}StripTrackParticles'

    trackKey = f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}Tracks'
    particleKey = f'SiSPSeedSegments{flags.Tracking.ActiveConfig.extension}TrackParticles'

   
    if processPixels:
        # Create track parameters from pixel seeds
        from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg
        acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(flags,
                                                    name = f'{flags.Tracking.ActiveConfig.extension}PixelSeedsToTrackParamsAlg',
                                                    extension = flags.Tracking.ActiveConfig.extension,
                                                    InputSeedContainerKey = seedKeyPixels,
                                                    OutputTrackParamsCollectionKey = paramsKeyPixels))


        # Convert pixel seed to Acts track
        acc.merge(ActsSeedToTrackCnvAlgCfg(flags,
                                           name=f"{flags.Tracking.ActiveConfig.extension}PixelSeedToTrackCnvAlg",
                                           EstimatedTrackParametersKey = [paramsKeyPixels],
                                           SeedContainerKey = [seedKeyPixels],
                                           ACTSTracksLocation = trackKeyPixels))

        # Truth
        if flags.Tracking.doTruth:
            from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{trackKeyPixels}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = trackKeyPixels,
                                                        AssociationMapOut = f"{trackKeyPixels}ToTruthParticleAssociation"))

            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{trackKeyPixels}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{trackKeyPixels}ToTruthParticleAssociation"))
            
        # Track Particle creation and persistification
        # - input track collection: trackKeyPixels
        # - output track particle collection: particleKeyPixels
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
                                                    OutputTrackParamsCollectionKey = paramsKeyStrips))

        # Convert strip seed to Acts track   
        acc.merge(ActsSeedToTrackCnvAlgCfg(flags, 
                                           name=f"{flags.Tracking.ActiveConfig.extension}StripSeedToTrackCnvAlg",
                                           EstimatedTrackParametersKey = [paramsKeyStrips],
                                           SeedContainerKey = [seedKeyStrips],
                                           ACTSTracksLocation = trackKeyStrips))

        # Truth
        if flags.Tracking.doTruth:
            from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
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
        from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
        acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
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


def ActsSeedToTrackCnvAlgCfg(flags,
                             name: str = "ActsSeedToTrackCnvAlg",
                             **kwargs: dict) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  kwargs.setdefault('SeedContainerKey', [])
  kwargs.setdefault('EstimatedTrackParametersKey', [])
  kwargs.setdefault('ACTSTracksLocation', f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}Tracks')

  if 'TrackingGeometryTool' not in kwargs:
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault('TrackingGeometryTool', acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))

  acc.addEventAlgo(CompFactory.ActsTrk.SeedToTrackCnvAlg(name, **kwargs))
  return acc

