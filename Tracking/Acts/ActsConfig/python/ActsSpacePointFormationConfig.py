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

    acc.setPrivateTools(CompFactory.ActsTrk.CoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsPixelSpacePointFormationCfg(flags,
                                    name: str = "ActsPixelSpacePointFormation",
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    if 'SpacePointFormationTool' not in kwargs:
        kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsPixelSpacePointToolCfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsPixelSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsPixelSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsStripSpacePointFormationCfg(flags,
                                    name: str = "ActsStripSpacePointFormation",
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

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
        acc.merge(ActsPixelSpacePointFormationCfg(flags,
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
        
        acc.merge(ActsStripSpacePointFormationCfg(flags,
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
        
        acc.merge(ActsStripSpacePointFormationCfg(flags,
                                                  name="ActsConversionStripSpacePointFormation",
                                                  StripClusters="ITkConversionStripClusters_InView" if flags.Acts.useCache else "ITkConversionStripClusters",
                                                  StripSpacePoints="ITkConversionStripSpacePoints",
                                                  StripOverlapSpacePoints="ITkConversionStripOverlapSpacePoints"))

    # Analysis extensions
    if flags.Acts.doAnalysis:
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripSpacePointAnalysisAlgCfg, ActsStripOverlapSpacePointAnalysisAlgCfg
            acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags,
                                                        name="ActsConversionStripSpacePointAnalysisAlg",
                                                        extension="ActsConversion",
                                                        SpacePointContainerKey="ITkConversionStripSpacePoints"))
            acc.merge(ActsStripOverlapSpacePointAnalysisAlgCfg(flags,
                                                               name="ActsConversionStripOverlapSpacePointAnalysisAlg",
                                                               extension="ActsConversion",
                                                               SpacePointContainerKey="ITkConversionStripOverlapSpacePoints"))
        
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
