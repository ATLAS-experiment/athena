# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsHGTDTrackExtensionAlgCfg(flags,
                                 name: str = "ActsHGTDTrackExtensionAlg",
                                 *,
                                 enableTrackStatePrinter: bool = False,
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("UncalibratedMeasurementContainerKeys", ["HGTD_Clusters"])

    if flags.Acts.doMonitoring and "MonTools" not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsHGTDTrackExtensionMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsHGTDTrackExtensionMonitoringCfg(flags)))

    if enableTrackStatePrinter and 'TrackStatePrinter' not in kwargs:
        from ActsConfig.ActsTrackFindingConfig import ActsTrackStatePrinterToolCfg
        kwargs.setdefault("TrackStatePrinter", acc.popToolsAndMerge(ActsTrackStatePrinterToolCfg(flags)))

    kwargs.setdefault('ACTSTracksLocation', 'HgtdTracks')
    kwargs.setdefault("etaBins", [2.0,2.6,2.8,3.0,3.2,3.4,3.6,3.8,3.9,999.0])

    #Measurement selector
    kwargs.setdefault("chi2CutOff", [15, 15, 15, 20, 10, 20, 20, 20, 50])
    kwargs.setdefault("chi2OutlierCutOff", [15, 15, 15, 10, 20, 20, 20, 20, 50])
    kwargs.setdefault("numMeasurementsCutOff", [3])

    #Track selector
    #kwargs.setdefault('maxChi2', [25,25,25,25,25,70,70,70])
    kwargs.setdefault('maxChi2', [500])
    kwargs.setdefault('maxOutliers', [20])
        
    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTrackExtensionAlg(name, **kwargs))
    return acc

def HGTDTruthTrackDecorationAlgCfg(flags,
                                   name: str = "HGTDTruthTrackDecorationAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")

    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTruthTrackDecorationAlg(name, **kwargs))
    return acc

