# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
    kwargs.setdefault("etaBins", [2.0, 3.0, 3.2, 3.4, 999.0])

    #Measurement selector
    # in sigma of a 1D Gaussian; for the 3D HGTD measurements these are the chi2 cuts 15, 20, 10, 20
    kwargs.setdefault("chi2CutOffSigma", [ 3.1187, 3.7603, 2.3541, 3.7603])
    kwargs.setdefault("chi2OutlierCutOffSigma", [ 3.1187, 3.7603, 2.3541, 3.7603])
    kwargs.setdefault("numMeasurementsCutOff", [3])

    #Track selector
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

