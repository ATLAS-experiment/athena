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
    kwargs.setdefault("etaBins", [0])

    #Measurement selector
    kwargs.setdefault("chi2CutOff", [30])
    kwargs.setdefault("chi2OutlierCutOff", [15])
    kwargs.setdefault("numMeasurementsCutOff", [3])

    #Track selector
    kwargs.setdefault('maxChi2', [30])
    kwargs.setdefault('maxOutliers', [2])
        
    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTrackExtensionAlg(name, **kwargs))
    return acc

def HGTDTruthTrackDecorationAlgCfg(flags,
                                   name: str = "HGTDTruthTrackDecorationAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackParticleContainerName", "InDetTrackParticles")

    acc.addEventAlgo(CompFactory.ActsTrk.HGTDTruthTrackDecorationAlg(name, **kwargs))
    return acc

