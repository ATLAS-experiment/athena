# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsPixelSpacePointToolCfg(flags,
                               name: str = "ActsPixelSpacePointTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsStripSpacePointToolCfg(flags,
                               name: str = "ActsStripSpacePointTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    acc.setPrivateTools(CompFactory.ActsTrk.StripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsCoreStripSpacePointToolCfg(flags,
                                   name: str = "ActsCoreStripSpacePointTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    if 'ConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault("ConverterTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault('TrackingGeometryTool', acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
        
    acc.setPrivateTools(CompFactory.ActsTrk.CoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsPixelSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsPixelSpacePointPreparationAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsPixelSpacePointPreparationMonitoringTool")))

    acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    return acc

def ActsStripSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsStripSpacePointPreparationAlg",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsStripSpacePointPreparationMonitoringTool")))

    acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    return acc

def ActsPixelSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsPixelSpacePointFormationAlg",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    kwargs.setdefault('PixelClusters', 'ITkPixelClusters')
    kwargs.setdefault('PixelSpacePoints', 'ITkPixelSpacePoints')
    
    if 'SpacePointFormationTool' not in kwargs:
        kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsPixelSpacePointToolCfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsPixelSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsPixelSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsStripSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsStripSpacePointFormationAlg",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault('StripClusters', 'ITkStripClusters')
    kwargs.setdefault('StripSpacePoints', 'ITkStripSpacePoints')
    kwargs.setdefault('StripOverlapSpacePoints', 'ITkStripOverlapSpacePoints')
    
    if 'SpacePointFormationTool' not in kwargs:
        from ActsConfig.ActsConfigFlags import SpacePointStrategy
        if flags.Acts.SpacePointStrategy is SpacePointStrategy.ActsCore:
            kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsCoreStripSpacePointToolCfg(flags)))
        else:
            kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsStripSpacePointToolCfg(flags)))

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsStripSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsStripSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.StripSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsMainSpacePointFormationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,
                                                     PixelClusters = "ITkPixelClusters_InView" if flags.Acts.useCache else "ITkPixelClusters"))
    if flags.Detector.EnableITkStrip and not flags.Tracking.doITkFastTracking:
        # Need to schedule this here in case the Athena space point formation is not schedule
        # This is because as of now requires at least ITkSiElementPropertiesTableCondAlgCfg
        # This may be because the current strip space point formation algorithm is not using Acts
        # May be not necessary once the Acts-based strip space point maker is ready
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc.merge(ITkStripReadoutGeometryCfg(flags))
        
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))
        
        from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
        acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
        
        acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
                                                     StripClusters = "ITkStripClusters_InView" if flags.Acts.useCache else "ITkStripClusters"))

    # Analysis extensions
    if flags.Acts.doAnalysis:
        if flags.Detector.EnableITkPixel:
            from ActsConfig.ActsAnalysisConfig import ActsPixelSpacePointAnalysisAlgCfg
            acc.merge(ActsPixelSpacePointAnalysisAlgCfg(flags))
        if flags.Detector.EnableITkStrip and not flags.Tracking.doITkFastTracking:
            from ActsConfig.ActsAnalysisConfig import ActsStripSpacePointAnalysisAlgCfg, ActsStripOverlapSpacePointAnalysisAlgCfg
            acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags))
            acc.merge(ActsStripOverlapSpacePointAnalysisAlgCfg(flags))

    return acc

def ActsConversionSpacePointFormationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Detector.EnableITkStrip:
        # For the time being, while we wait for the cache mechanism to be available also for space points,
        # we do the following:
        #   - offline: we schedule the Preparation Algorithms and rely of the space point collections created in the main pass
        #   - online (i.e. with cache): we run space point formation and pass the output directly to seeding

        if not flags.Acts.useCache:
            acc.merge(ActsStripSpacePointPreparationAlgCfg(flags,
                                                           name = "ActsConversionStripSpacePointPreparationAlg",
                                                           RoIs = "ActsConversionRegionOfInterest",
                                                           InputCollection = "ITkStripSpacePoints",
                                                           OutputCollection = "ITkConversionStripSpacePoints"))
            
        else:            
            # Need to schedule this here in case the Athena space point formation is not schedule
            # This is because as of now requires at least ITkSiElementPropertiesTableCondAlgCfg
            # This may be because the current strip space point formation algorithm is not using Acts
            # May be not necessary once the Acts-based strip space point maker is ready            
            from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
            acc.merge(ITkStripReadoutGeometryCfg(flags))
            
            from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
            acc.merge(BeamSpotCondAlgCfg(flags))
            
            from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
            acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
            
            acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
                                                         name="ActsConversionStripSpacePointFormation",
                                                         StripClusters="ITkConversionStripClusters_InView",
                                                         StripSpacePoints="ITkConversionStripSpacePoints",
                                                         ProcessOverlapForStrip=False))
            
    # Analysis extensions
    if flags.Acts.doAnalysis:
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripSpacePointAnalysisAlgCfg
            acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags,
                                                        name="ActsConversionStripSpacePointAnalysisAlg",
                                                        extension="ActsConversion",
                                                        SpacePointContainerKey="ITkConversionStripSpacePoints"))
        
    return acc

def ActsSpacePointFormationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Acts Main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        acc.merge(ActsMainSpacePointFormationCfg(flags))
    # Acts Conversion pass
    elif flags.Tracking.ActiveConfig.extension == "ActsConversion":
        acc.merge(ActsConversionSpacePointFormationCfg(flags))
    # Any other pass -> Validation mainly
    else:
        acc.merge(ActsMainSpacePointFormationCfg(flags))

    return acc
