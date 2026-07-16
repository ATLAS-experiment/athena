
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SeedingStrategy
import AthenaCommon.SystemOfUnits as Units
from ActsInterop import UnitConstants


# Tools

def isdet(flags,
          *,
          pixel: list = None,
          strip: list = None,
          hgtd: list = None,
          noStrip: bool = False) -> list:
    keys = []
    if flags.Detector.EnableITkPixel and pixel is not None:
        keys += pixel
    if flags.Detector.EnableITkStrip and strip is not None and not noStrip:
        keys += strip
    if flags.Acts.useHGTDClusterInTrackFinding and hgtd is not None:
        keys += hgtd
    return keys

def seedOrder(flags,
          *,
          pixel: list = None,
          strip: list = None) -> list:
    keys = isdet(flags, pixel=pixel, strip=strip)
    if flags.Acts.useStripSeedsFirst:
        keys.reverse()
    return keys

def ActsTrackStatePrinterToolCfg(flags,
                                 name: str = "ActsTrackStatePrinterTool",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    kwargs.setdefault("InputSpacePoints", isdet(flags, noStrip=isFastPrimaryPass(flags),
                                                pixel=['ITkPixelSpacePoints_Cached'] if flags.Acts.useCache else ['ITkPixelSpacePoints'],
                                                strip=['ITkStripSpacePoints_Cached', 'ITkStripOverlapSpacePoints_Cached'] if flags.Acts.useCache else ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints']))

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinterTool(name, **kwargs))
    return acc

# ACTS only algorithm

def ActsMainTrackFindingAlgCfg(flags,
                               name: str = "ActsTrackFindingAlg",
                               **kwargs) -> ComponentAccumulator:
    def tolist(c):
        return c if isinstance(c, list) else [c]

    acc = ComponentAccumulator()

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsGeometryContextAlgCfg(flags))
    acc.merge(ActsTrackingGeometrySvcCfg(flags))

    from ActsConfig.ActsGeometryConfig import ActsVolumeIdToDetectorCollectionMappingAlgCfg
    # Remove HGTD Volumes from the propagation unless we need it
    if not flags.Acts.useHGTDClusterInTrackFinding:
        # HGTD has volume id:
        # 2 for negative endcap
        # 25 for positive endcap
        kwargs.setdefault('EndOfTheWorldVolumeIds', [2, 25])
    
    acc.merge( ActsVolumeIdToDetectorCollectionMappingAlgCfg(flags) )
    kwargs.setdefault("ActsVolumeIdToDetectorElementCollectionMapKey", "VolumeIdToDetectorElementCollectionMap")

    if flags.Detector.EnableITkPixel:
        from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementStatusAlgCfg
        acc.merge(ITkPixelDetectorElementStatusAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripDetectorElementStatusAlgCfg
        acc.merge(ITkStripDetectorElementStatusAlgCfg(flags))
    kwargs.setdefault("DetElStatus", seedOrder(flags, pixel=["ITkPixelDetectorElementStatus"], strip=["ITkStripDetectorElementStatus"]))

    # Seed labels and collections.
    # These 3 lists must match element for element, reversed if flags.Acts.useStripSeedsFirst is True.
    # Maybe it is best to start with strips where the occupancy is lower.
    kwargs.setdefault("SeedLabels", seedOrder(flags, pixel=["PPP"], strip=["SSS"]))
    kwargs.setdefault("SeedContainerKeys", seedOrder(flags, pixel=["ActsPixelSeeds"], strip=["ActsStripSeeds"]))
    if flags.Acts.Tracks.doAnalysis:
        kwargs.setdefault("SeedDestiny", [f'{seedkey}Destiny' for seedkey in kwargs["SeedContainerKeys"]])

    kwargs.setdefault("UncalibratedMeasurementContainerKeys", isdet(flags, pixel=["ITkPixelClusters_Cached" if flags.Acts.useCache else "ITkPixelClusters"], strip=["ITkStripClusters_Cached" if flags.Acts.useCache else "ITkStripClusters"], hgtd=["HGTD_Clusters"]))

    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks')

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    kwargs.setdefault("seedMeasOffset", 1)

    # Ambi strategy 0 means do the ambiguity resolution outside the track finding.
    kwargs.setdefault("ambiStrategy", flags.Acts.AmbiguitySolverMode.value)
    
    if (not flags.Acts.doAmbiguityResolution) :
        kwargs.setdefault("MaximumSharedHits", 3)
        kwargs.setdefault("MaximumIterations", 10000)
        kwargs.setdefault("NMeasurementsMin", 7)
    
    kwargs.setdefault("doTwoWay", flags.Acts.doTwoWayCKF)
    # drop the track states on material-only surfaces: nothing downstream reads them, and the CKF is faster without
    kwargs.setdefault("recordMaterialStates", False)
    kwargs.setdefault("autoReverseSearch", flags.Tracking.ActiveConfig.autoReverseSearch)
    # forceTrackOnSeed isn't effective with secondary passes, which will have removed most/all of the seed measurements from the measurement containers.
    kwargs.setdefault("forceTrackOnSeed", flags.Acts.forceTrackOnSeed and not flags.Tracking.ActiveConfig.isSecondaryPass)

    # Borrow many settings from flags.Tracking.ActiveConfig, normally initialised in createITkTrackingPassFlags() at
    # https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkConfig/python/TrackingPassFlags.py#L121

    # bins in |eta|, used for both MeasurementSelectorConfig and TrackSelector::EtaBinnedConfig
    if flags.Detector.GeometryITk:
        kwargs.setdefault("etaBins", flags.Tracking.ActiveConfig.etaBins)
    # new default chi2 cuts optimise efficiency vs speed. Set same value as Athena's Xi2maxNoAdd.
    kwargs.setdefault("chi2CutOff", flags.Tracking.ActiveConfig.Xi2max)
    kwargs.setdefault("chi2OutlierCutOff", flags.Tracking.ActiveConfig.Xi2maxNoAdd)

    kwargs.setdefault("branchStopperPtMinFactor", 0.9)
    kwargs.setdefault("branchStopperAbsEtaMaxExtra", 0.1)
    if not flags.Tracking.ActiveConfig.isLargeD0:   # consider seedRefitPtMinFactor later, with other LRT optimisations
        kwargs.setdefault("seedRefitPtMinFactor", 0.9)

    # Loosen the requirement on the minimum number of measurements on track candidate
    # during track finding for tracks above a certain eta
    kwargs.setdefault("branchStopperMeasCutReduce", flags.Acts.branchStopperMeasCutReduce)
    kwargs.setdefault("branchStopperAbsEtaMeasCut", flags.Acts.branchStopperAbsEtaMeasCut)

    kwargs.setdefault("numMeasurementsCutOff", [1])

    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized by abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins", [eta/10. for eta in range(5, 40, 5)]) # eta 0.0 - 4.0 in steps of 0.5

    kwargs.setdefault("absEtaMax", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("ptMin", [p / Units.GeV * UnitConstants.GeV for p in tolist(flags.Tracking.ActiveConfig.minPT)])
    # z0 cut is the same for all eta bins. I use the size of the eta bins limits minus one to find the number of bins.
    kwargs.setdefault("z0Min", [-flags.Tracking.ActiveConfig.maxZImpactSeed / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("z0Max", [ flags.Tracking.ActiveConfig.maxZImpactSeed / Units.mm * UnitConstants.mm for etabin in flags.Tracking.ActiveConfig.etaBins[:-1]])
    kwargs.setdefault("d0Min", [-d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpact)])
    kwargs.setdefault("d0Max", [ d0 / Units.mm * UnitConstants.mm for d0 in tolist(flags.Tracking.ActiveConfig.maxPrimaryImpact)])
    kwargs.setdefault("minMeasurements", tolist(flags.Tracking.ActiveConfig.minClusters))
    kwargs.setdefault("maxHoles", tolist(flags.Tracking.ActiveConfig.maxHoles))
    kwargs.setdefault("minPixelHits", tolist(flags.Tracking.ActiveConfig.minPixel))
    kwargs.setdefault("maxPixelHoles", tolist(flags.Tracking.ActiveConfig.maxPixelHoles))
    kwargs.setdefault("maxStripHoles", tolist(flags.Tracking.ActiveConfig.maxSctHoles))
    # The shared hits are not calculated until *after* the track selection, so maxSharedHits is not used.
    # Even if that were not the case, we need the ambiguity solver to decide which track to drop.
    ### kwargs.setdefault("maxSharedHits", tolist(flags.Tracking.ActiveConfig.maxShared))

    # GBTS produces much purer seeds, so the branch stopper selections aren't needed with GBTS seeds.
    if flags.Tracking.ActiveConfig.SeedingStrategy not in [
            SeedingStrategy.GbtsFtf, SeedingStrategy.Gbts]:
        kwargs.setdefault("ptMinMeasurements", seedOrder(flags, pixel=[3], strip=[6]))
        kwargs.setdefault("absEtaMaxMeasurements", seedOrder(flags, pixel=[3], strip=[999999]))

    if 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg

        # set TrackParamsEstimationTool in case not defined by caller
        tpe_tool_kwargs = {}
        if flags.Tracking.ActiveConfig.isLargeD0:
            tpe_tool_kwargs["allowPropagatorFailure"] = True
        tpe_tool_kwargs["stripCalibrationIterations"] = flags.Acts.stripCalibrationIterations
        tpe = acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags, **tpe_tool_kwargs))

        kwargs.setdefault('TrackParamsEstimationTool', seedOrder(flags, pixel=[tpe], strip=[tpe]))

    if flags.Acts.doPrintTrackStates and 'TrackStatePrinter' not in kwargs:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsTrackStatePrinterToolCfg(flags)),
        )
 
    if 'PixelCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType

        if flags.Beam.Type is not BeamType.Cosmics and flags.Acts.PixelCalibrationStrategy.usesCalibration():
            from ActsConfig.ActsMeasurementCalibrationConfig import ActsPixelCalibrationToolCfg

            kwargs.setdefault(
                'PixelCalibrator',
                acc.popToolsAndMerge(ActsPixelCalibrationToolCfg(flags))
            )

    if 'StripCalibrator' not in kwargs:
        from AthenaConfiguration.Enums import BeamType

        if flags.Beam.Type is not BeamType.Cosmics and flags.Acts.StripCalibrationStrategy.usesCalibration():
            from ActsConfig.ActsMeasurementCalibrationConfig import ActsStripCalibrationToolCfg

            kwargs.setdefault(
                'StripCalibrator',
                acc.popToolsAndMerge(ActsStripCalibrationToolCfg(flags))
            )

        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc



def ActsTrackFindingCfg(flags,
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Define Uncalibrated Measurement keys
    dataPrepPrefix = f'{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}'
    if not flags.Tracking.ActiveConfig.isSecondaryPass:
        dataPrepPrefix = ''
    pixelClusters = f'ITk{dataPrepPrefix}PixelClusters'
    stripClusters = f'ITk{dataPrepPrefix}StripClusters'    
    hgtdClusters = f'{dataPrepPrefix}HGTD_Clusters'
    # If cache is activated the keys have "_Cached" as postfix
    if flags.Acts.useCache:
        pixelClusters += '_Cached'
        stripClusters += '_Cached'
    # Consider case detectors are not active

    # Understand what are the seeds we need to consider
    pixelSeedLabels = ['PPP']
    stripSeedLabels = ['SSS']
    # Conversion and LRT do not process pixel seeds
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass
    if flags.Tracking.ActiveConfig.extension == 'ActsConversion' or flags.Tracking.ActiveConfig.isLargeD0:
        pixelSeedLabels = None
    # Main pass does not process strip seeds in the fast tracking configuration
    elif isFastPrimaryPass(flags):
        stripSeedLabels = None

    # Now set the seed and estimated parameters keys accordingly
    pixelSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}PixelSeeds']
    stripSeedKeys = [f'{flags.Tracking.ActiveConfig.extension}StripSeeds']
    pixelDetElements = ['ITkPixelDetectorElementCollection']
    stripDetElements = ['ITkStripDetectorElementCollection']

    tpe_tool_kwargs = {}
    if flags.Tracking.ActiveConfig.isLargeD0:
        tpe_tool_kwargs["allowPropagatorFailure"] = True
    tpe_tool_kwargs["stripCalibrationIterations"] = flags.Acts.stripCalibrationIterations

    if pixelSeedLabels is None:
        pixelSeedKeys = None
        pixelDetElements = None
        pixelTpe = None
    elif 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
        pixelTpe = [acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags, "PixelTrackParamsEstimationTool", **tpe_tool_kwargs))]

    if stripSeedLabels is None:
        stripSeedKeys = None
        stripDetElements = None
        stripTpe = None
    elif 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
        if flags.Tracking.ActiveConfig.isLargeD0 and flags.Acts.LrtStripSeedRefit:
            tpe_tool_kwargs["refitSeeds"] = True
        stripTpe = [acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags, "StripTrackParamsEstimationTool", **tpe_tool_kwargs))]

    kwargs.setdefault("TrackParamsEstimationTool", seedOrder(flags, pixel=pixelTpe, strip=stripTpe))
    kwargs.setdefault('ACTSTracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('UncalibratedMeasurementContainerKeys', isdet(flags, pixel=[pixelClusters], strip=[stripClusters], hgtd=[hgtdClusters]))
    kwargs.setdefault('SeedLabels', seedOrder(flags, pixel=pixelSeedLabels, strip=stripSeedLabels))
    kwargs.setdefault('SeedContainerKeys', seedOrder(flags, pixel=pixelSeedKeys, strip=stripSeedKeys))

    acc.merge(ActsMainTrackFindingAlgCfg(flags,
                                         name=f"{flags.Tracking.ActiveConfig.extension}TrackFindingAlg",
                                         **kwargs))

    # Analysis extensions
    if flags.Acts.Tracks.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
        acc.merge(ActsTrackAnalysisAlgCfg(flags,
                                          name=f"{flags.Tracking.ActiveConfig.extension}TrackAnalysisAlg",
                                          TracksLocation=f"{flags.Tracking.ActiveConfig.extension}Tracks"))

        # Seed To Track Monitoring
        DetectorElementsKeys=seedOrder(flags, pixel=pixelDetElements, strip=stripDetElements)
        if len(kwargs["SeedContainerKeys"]) != len(DetectorElementsKeys):
            raise AttributeError("SeedContainerKeys and DetectorElementsKeys must have same size")

        for i in range(0, len(kwargs["SeedContainerKeys"])):
            seedKey = kwargs["SeedContainerKeys"][i]
            detElKey = DetectorElementsKeys[i]

            # make seed params
            from ActsConfig.ActsAnalysisConfig import ActsBaseSeedsToTrackParamsAlgCfg
            acc.merge(ActsBaseSeedsToTrackParamsAlgCfg(flags,
                                                       name = f'{seedKey}SeedsToTrackParamsAlg',
                                                       InputSeedContainerKey = seedKey,
                                                       DetectorElementsKey = detElKey,
                                                       OutputTrackParamsCollectionKey = f'{seedKey}Params'))

            from ActsConfig.ActsAnalysisConfig import ActsSeedToTrackAnalysisAlgCfg
            acc.merge(ActsSeedToTrackAnalysisAlgCfg(flags,
                                                    name = f'{seedKey}ToTrackAnalysisAlg',
                                                    InputSeedCollection = seedKey,
                                                    InputTrackParamsCollection = f'{seedKey}Params',
                                                    InputDestinyCollection = f'{seedKey}Destiny'))

    # Persistification
    if flags.Acts.EDM.PersistifyTracks or flags.Output.doWriteESD:
        trackColl = kwargs['ACTSTracksLocation']
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        acc.merge(ActsToXAODTrackConverterAlgCfg(flags,
                                                 name = f'{trackColl}ToXAODConverterAlg',
                                                 InputActsTracksLocation = trackColl,
                                                 OutputActsTracksLocation = trackColl))

        prefix = f"{flags.Tracking.ActiveConfig.extension}"
        from ActsConfig.ActsPersistificationConfig import PersistifyTracks
        acc.merge(PersistifyTracks(flags,
                                   extensions=[prefix]))

    return acc


def ActsTrackFindingGNNCfg(flags, **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # This is added in the seeding step of the CKF chain...
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    
    # Adopt standard convention
    kwargs.setdefault('ACTSTracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")

    kwargs.setdefault("moduleMapPath", flags.Acts.GNN.ModuleMapPath)
    kwargs.setdefault("gnnPath", flags.Acts.GNN.ModelPath)
    kwargs.setdefault("numTrtContexts", flags.Acts.GNN.NumTrtContexts)
    kwargs.setdefault("maxGpuInstances", flags.Acts.GNN.MaxGpuInstances)
    kwargs.setdefault("varianceInflation", flags.Acts.GNN.VarianceInflation)
    kwargs.setdefault("tightSeeds", flags.Acts.GNN.TightSeeds)
    kwargs.setdefault("edgeCut", flags.Acts.GNN.EdgeCut)
    kwargs.setdefault("minCandidateMeasurements", flags.Acts.GNN.MinCandidateMeasurements)
    kwargs.setdefault("minDeltaR", flags.Acts.GNN.MinDeltaR)
    kwargs.setdefault("relaxCentralHoleSel", flags.Acts.GNN.RelaxCentralHoleSel)
    kwargs.setdefault("relaxMeasurementSel", flags.Acts.GNN.RelaxMeasurementSel)
    kwargs.setdefault("offlineZ0Sel", flags.Acts.GNN.OfflineZ0Sel)

    # Wire parameter estimation and fitter tools like the main Acts path
    if 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
        kwargs.setdefault('TrackParamsEstimationTool', acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags)))

    # The fitter tool is used in the GNN track finding to fit the track candidates after the GNN has selected the measurements.
    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
        kwargs.setdefault('FitterTool', acc.popToolsAndMerge(ActsFitterCfg(flags, 
                                                                           ReverseFilteringPt=0, 
                                                                           OutlierChi2Cut=float('inf'))))

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsGeometryContextAlgCfg(flags))
    acc.merge(ActsTrackingGeometrySvcCfg(flags))

    acc.addEventAlgo(
        CompFactory.ActsTrk.TrackFindingGNNAlg("TrackFindingGNNAlg", **kwargs)
    )

    return acc




def ActsMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsScoreBasedAmbiguityResolutionAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ActsResolvedTracks')
    kwargs.setdefault('MinScore',1.0)
    kwargs.setdefault('MinScoreSharedTracks', 1.0)
    kwargs.setdefault('MaxSharedTracksPerMeasurement', 20)
    kwargs.setdefault('MaxShared', 5)
    kwargs.setdefault('MinUnshared', 5)
    kwargs.setdefault('UseAmbiguityScoring', True)
    kwargs.setdefault('jsonFileName', 'ActsAmbiguityConfig.json')

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.ScoreBasedAmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsMainAmbiguityResolutionAlgCfg(flags,
                                      name: str = "ActsAmbiguityResolutionAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ActsResolvedTracks')
    kwargs.setdefault('MaximumSharedHits', 3)
    kwargs.setdefault('MaximumIterations', 10000)
    kwargs.setdefault('NMeasurementsMin', 7)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc


def ActsAmbiguityResolutionCfg(flags,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TracksLocation', f"{flags.Tracking.ActiveConfig.extension}Tracks")
    kwargs.setdefault('ResolvedTracksLocation', f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks")
    from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy
            
    if flags.Acts.AmbiguitySolverStrategy is AmbiguitySolverStrategy.ScoreBased:
        acc.merge(ActsMainScoreBasedAmbiguityResolutionAlgCfg(flags,
                                                    name=f"{flags.Tracking.ActiveConfig.extension}ScoreBasedAmbiguityResolutionAlg",
                                                    **kwargs))
    else:
        acc.merge(ActsMainAmbiguityResolutionAlgCfg(flags,
                                                    name=f"{flags.Tracking.ActiveConfig.extension}AmbiguityResolutionAlg",
                                                    **kwargs))
    # Analysis extensions
    if flags.Acts.Tracks.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsTrackAnalysisAlgCfg
        acc.merge(ActsTrackAnalysisAlgCfg(flags,
                                          name=f"{flags.Tracking.ActiveConfig.extension}ResolvedTrackAnalysisAlg",
                                          TracksLocation=f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"))

    # Persistification
    if flags.Acts.EDM.PersistifyTracks or flags.Output.doWriteESD:
        trackColl = kwargs['ResolvedTracksLocation']
        from ActsConfig.ActsEventCnvConfig import ActsToXAODTrackConverterAlgCfg
        acc.merge(ActsToXAODTrackConverterAlgCfg(flags,
                                                 name = f'{trackColl}ToXAODConverterAlg',
                                                 InputActsTracksLocation = trackColl,
                                                 OutputActsTracksLocation = trackColl))

        prefix = f"{flags.Tracking.ActiveConfig.extension}Resolved"
        from ActsConfig.ActsPersistificationConfig import PersistifyTracks
        acc.merge(PersistifyTracks(flags,
                                   extensions=[prefix]))

    return acc


