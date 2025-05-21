# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SeedingStrategy
from ActsConfig.ActsUtilities import extractChildKwargs
from ActsInterop import UnitConstants
from AthenaCommon.Utils.unixtools import find_datafile

def ActsGbts2SeedingTrigToolCfg(flags,name: str = "Gbts2ActsSeedingTool", **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  if "layerNumberTool" not in kwargs:
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import ITkTrigL2LayerNumberToolCfg
    ntargs = {"UseNewLayerScheme" : True}
    kwargs.setdefault("layerNumberTool",acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags,**ntargs)))
  
  kwargs.setdefault("DoPhiFiltering", False) #no phi-filtering for full-scan tracking
  kwargs.setdefault("UseBeamTilt", False)

  isLRT=flags.Tracking.ActiveConfig.extension == "LargeD0"
  
  kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT[0])
  kwargs.setdefault("MaxGraphEdges", 1500000)   # do we want ITkTrigTrackSeedingToolStandaloneCfg definition: if flags.Acts.GbtsConnectionTableVersion == 0 else 1800000)
  if flags.Acts.GbtsConnectionTableVersion == 0:
    kwargs.setdefault("MatchBeforeCreate", True)
  kwargs.setdefault("ConnectionFileName",
                    "binTables_ITK_RUN4_LRT.txt" if isLRT else "binTables_ITK_RUN4.txt")

  acc.setPrivateTools(CompFactory.Gbts2ActsSeedingTool(name, **kwargs))

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
    kwargs.setdefault("rMax", 320. * UnitConstants.mm)
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
    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name, **kwargs))
    return acc

def ActsFastPixelSeedingToolCfg(flags,
                                name: str = "ActsFastPixelSeedingTool",
                                **kwargs) -> ComponentAccumulator:
    ## Additional cuts for fast seed configuration
    kwargs.setdefault("minPt", 1000 * UnitConstants.MeV)
    kwargs.setdefault("collisionRegionMin", -150 * UnitConstants.mm)
    kwargs.setdefault("collisionRegionMax", 150 * UnitConstants.mm)
    kwargs.setdefault("maxPhiBins", 200)
    kwargs.setdefault("gridRMax", 250 * UnitConstants.mm)
    kwargs.setdefault("deltaRMax", 200 * UnitConstants.mm)
    kwargs.setdefault("zBinsCustomLooping" , [3, 11, 4, 10, 7, 5, 9, 6, 8])
    kwargs.setdefault("rRangeMiddleSP", [
             [40.0, 80.0],
             [40.0, 80.0],
             [40.0, 200.0],
             [70.0, 200.0],
             [70.0, 200.0],
             [70.0, 250.0],
             [70.0, 250.0],
             [70.0, 250.0],
             [70.0, 200.0],
             [70.0, 200.0],
             [40.0, 200.0],        
             [40.0, 80.0],
             [40.0, 80.0]])
    kwargs.setdefault("useVariableMiddleSPRange", False)
    kwargs.setdefault("useExperimentCuts", True)
    kwargs.setdefault("rMax", 320 * UnitConstants.mm)
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])

    return ActsPixelSeedingToolCfg(flags, name, **kwargs)

def ActsStripSeedingToolCfg(flags,
                            name: str = "ActsStripSeedingTool",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("doSeedQualitySelection", False)
    # For SpacePointGridConfig
    kwargs.setdefault("gridRMax" , 1000. * UnitConstants.mm)
    kwargs.setdefault("deltaRMax" , 600. * UnitConstants.mm)
    kwargs.setdefault("impactMax" , 20. * UnitConstants.mm)
    # For SeedfinderConfig
    kwargs.setdefault("rMax" , 1200. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinTopSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaZMax" , 900. * UnitConstants.mm)
    kwargs.setdefault("interactionPointCut" , False)
    kwargs.setdefault("zBinsCustomLooping" , [7, 8, 6, 9, 5, 10, 4, 11, 3, 12, 2])
    kwargs.setdefault("deltaRMiddleMinSPRange" , 30 * UnitConstants.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange" , 150 * UnitConstants.mm)
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
    kwargs.setdefault("zBinNeighborsBottom" , [(0,0),(0,1),(0,1),(0,1),(0,2),(0,1),(0,0),(-1,0),(-2,0),(-1,0),(-1,0),(-1,0),(0,0)])
    # Any other
    kwargs.setdefault("rBinEdges", [0, kwargs['rMax']])
        
    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name, **kwargs))
    return acc

def ActsPixelOrthogonalSeedingToolCfg(flags,
                                      name: str = "ActsPixelOrthogonalSeedingTool",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()    
    ## For ITkPixel, use default values for ActsTrk::OrthogonalSeedingTool
    acc.setPrivateTools(CompFactory.ActsTrk.OrthogonalSeedingTool(name, **kwargs))
    return acc

def ActsFastPixelOrthogonalSeedingToolCfg(flags,
                                          name: str = "ActsFastPixelOrthogonalSeedingTool", 
                                          **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkPixel, use default values for ActsTrk::OrthogonalSeedingTool

    ## Additional cuts for fast seed configuration
    kwargs.setdefault("minPt", 1000 * UnitConstants.MeV)
    kwargs.setdefault("collisionRegionMin", -150 * UnitConstants.mm)
    kwargs.setdefault("collisionRegionMax", 150 * UnitConstants.mm)
    kwargs.setdefault("useExperimentCuts", True)
    
    acc.setPrivateTools(CompFactory.ActsTrk.OrthogonalSeedingTool(name, **kwargs))
    return acc

def ActsStripOrthogonalSeedingToolCfg(flags,
                                      name: str = "ActsStripOrthogonalSeedingTool",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("impactMax" , 20. * UnitConstants.mm)
    kwargs.setdefault('rMax', 1200. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinTopSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaZMax" , 900. * UnitConstants.mm)
    kwargs.setdefault("interactionPointCut" , False)
    kwargs.setdefault("impactWeightFactor" , 1.)
    kwargs.setdefault("compatSeedLimit" , 4)
    kwargs.setdefault("seedWeightIncrement" , 10100.)
    kwargs.setdefault("numSeedIncrement" , 1.)
    kwargs.setdefault("seedConfirmationInFilter" , False)
    kwargs.setdefault("maxSeedsPerSpMConf" , 100)
    kwargs.setdefault("maxQualitySeedsPerSpMConf" , 100)
    kwargs.setdefault("useDeltaRorTopRadius" , False)
    kwargs.setdefault("rMinMiddle", 33. * UnitConstants.mm)
    kwargs.setdefault("rMaxMiddle", 1200. * UnitConstants.mm)
    
    acc.setPrivateTools(CompFactory.ActsTrk.OrthogonalSeedingTool(name, **kwargs))
    return acc

def ActsPixelGbtsSeedingToolCfg(flags,
                                name: str = "ActsPixelGbtsSeedingTool", 
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkPixel, use default values for ActsTrk::GbtsSeedingTool
    kwargs.setdefault("ConnectorInputFile" , find_datafile("binTables_ITK_RUN4.txt"))

    kwargs.setdefault('PixelDetectorElements', 'ITkPixelDetectorElementCollection')

    acc.setPrivateTools(CompFactory.ActsTrk.GbtsSeedingTool(name = name, **kwargs))
    return acc

def ActsSiSpacePointsSeedMakerToolCfg(flags,
                                      name: str = 'ActsSiSpacePointsSeedMakerTool',
                                      **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str)

    acc = ComponentAccumulator()

    if flags.Tracking.ActiveConfig.extension == "ActsValidateConversionSeeds":
        kwargs.setdefault('useOverlapSpCollection', False)
    
    # Main properties
    kwargs.setdefault('usePixel', 
                      flags.Tracking.ActiveConfig.useITkPixel and
                      flags.Tracking.ActiveConfig.useITkPixelSeeding)
    kwargs.setdefault('useStrip',
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault('useOverlapSpCollection',
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault('ActsSpacePointsPixelName'    , "ITkPixelSpacePoints")
    kwargs.setdefault('ActsSpacePointsStripName'    , "ITkStripSpacePoints")
    kwargs.setdefault('ActsSpacePointsOverlapName'  , "ITkStripOverlapSpacePoints")

    
    # The code will need to use Trk::SpacePoint object for downstream Athena tracking
    # If we run this tool we have two options to retrieve this:
    #     (1) Have the Athena->Acts Space Point Converter scheduled beforehand
    #     (2) Have the Athena->Acts Cluster Converter scheduled beforehand
    # In case (1) the link xAOD -> Trk Space Point will be used to retrieve the Trk::SpacePoints
    # In case (2) the link xAOD -> InDet Cluster will be used to create the Trk::SpacePoints
    # If none of the above conditions are met, it means there is a misconfiguration of the algorithms
    useClusters = flags.Tracking.ActiveConfig.doAthenaToActsCluster and not flags.Tracking.ActiveConfig.doAthenaToActsSpacePoint
    kwargs.setdefault('useClustersForSeedConversion', useClusters)

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault('PRDtoTrackMap', (
            'ITkPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    # Acts Seed Tools
    # Do not overwrite if already present in `kwargs`
    seedTool_pixel = None
    if 'SeedToolPixel' not in kwargs:
        from InDetConfig.ITkActsHelpers import isFastPrimaryPass
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            if isFastPrimaryPass(flags):
                seedTool_pixel = acc.popToolsAndMerge(ActsFastPixelOrthogonalSeedingToolCfg(flags))
            else:
                seedTool_pixel = acc.popToolsAndMerge(ActsPixelOrthogonalSeedingToolCfg(flags))
        elif flags.Acts.SeedingStrategy is SeedingStrategy.Gbts:
            seedTool_pixel = acc.popToolsAndMerge(ActsPixelGbtsSeedingToolCfg(flags))
        elif flags.Acts.SeedingStrategy is SeedingStrategy.Gbts2:
            seedTool_pixel = acc.popToolsAndMerge(ActsGbts2SeedingTrigToolCfg(flags))
        else:
            if isFastPrimaryPass(flags):
                kwargs.setdefault("useFastTracking", True)
                seedTool_pixel = acc.popToolsAndMerge(ActsFastPixelSeedingToolCfg(flags))
            else:
                seedTool_pixel = acc.popToolsAndMerge(ActsPixelSeedingToolCfg(flags))

    seedTool_strip = None
    if 'SeedToolStrip' not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool_strip = acc.popToolsAndMerge(ActsStripOrthogonalSeedingToolCfg(flags))
        else:
            seedTool_strip = acc.popToolsAndMerge(ActsStripSeedingToolCfg(flags,
                                                                          rMax=flags.Tracking.ActiveConfig.radMax))

    kwargs.setdefault('SeedToolPixel', seedTool_pixel)
    kwargs.setdefault('SeedToolStrip', seedTool_strip)

    # Validation
    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault('WriteNtuple', True)
        HistService = CompFactory.THistSvc(Output = ["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"])
        acc.addService(HistService)

    acc.setPrivateTools(CompFactory.ActsTrk.SiSpacePointsSeedMaker(name, **kwargs))
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
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            if useFastTracking:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsFastPixelOrthogonalSeedingToolCfg(flags)))
            else:
                kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsPixelOrthogonalSeedingToolCfg(flags)))
        elif flags.Acts.SeedingStrategy is SeedingStrategy.Gbts:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsPixelGbtsSeedingToolCfg(flags)))
        elif flags.Acts.SeedingStrategy is SeedingStrategy.Gbts2:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsGbts2SeedingTrigToolCfg(flags)))
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

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
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
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsStripOrthogonalSeedingToolCfg(flags)))
        else:
            kwargs.setdefault('SeedTool', acc.popToolsAndMerge(ActsStripSeedingToolCfg(flags)))

    kwargs.setdefault('InputSpacePoints', ['ITkStripSpacePoints_Cached', 'ITkStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ActsStripSeeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkStripSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
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
    if flags.Tracking.ActiveConfig.extension in ["ActsConversion", "ActsLargeRadius"]:
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
                                                                                                   name=f'{flags.Tracking.ActiveConfig.extension}PixelSeedingTool',
                                                                                                   minPt=flags.Tracking.ActiveConfig.minPTSeed)))
    if processStrips and flags.Acts.SeedingStrategy is SeedingStrategy.Default:
        kwargs.setdefault('StripSeedingAlg.SeedTool', acc.popToolsAndMerge(ActsStripSeedingToolCfg(flags,
                                                                                                   name=f'{flags.Tracking.ActiveConfig.extension}StripSeedingTool',
                                                                                                   rMax=flags.Tracking.ActiveConfig.radMax)))
        
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

    from ActsConfig.ActsGeometryConfig import ActsDetectorElementToActsGeometryIdMappingAlgCfg
    acc.merge( ActsDetectorElementToActsGeometryIdMappingAlgCfg(flags) )
    
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
  kwargs.setdefault('DetectorElementToActsGeometryIdMapKey', 'DetectorElementToActsGeometryIdMap')

  kwargs.setdefault('SeedContainerKey', [])
  kwargs.setdefault('EstimatedTrackParametersKey', [])
  kwargs.setdefault('ACTSTracksLocation', f'SiSPTracksSeedSegments{flags.Tracking.ActiveConfig.extension}Tracks')

  if 'TrackingGeometryTool' not in kwargs:
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault('TrackingGeometryTool', acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))

  acc.addEventAlgo(CompFactory.ActsTrk.SeedToTrackCnvAlg(name, **kwargs))
  return acc

