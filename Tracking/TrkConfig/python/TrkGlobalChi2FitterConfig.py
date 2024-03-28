# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkGlobalChi2Fitter package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
import AthenaCommon.SystemOfUnits as Units

# The Global Chi2 is the main/reference general
# track fitter in ATLAS
# (at least for Run-1,Run-2,Run-3 ...)
# See:
# https://iopscience.iop.org/article/10.1088/1742-6596/119/3/032013
# Advantages:
# - It only needs an initial estimate of the track parameters
# - It can  solve the left/right ambiguities in drift circle hits.
# - Yields the scattering angles on the track (used in alignment).
# Main disadvantage
# - Needs to invert much larger matrices than the Kalman fitter.


#########################
# InDet configs
#########################
def InDetGlobalChi2FitterBaseCfg(
        flags, name='GlobalChi2FitterBase', **kwargs):
    acc = ComponentAccumulator()

    if 'TrackingGeometryReadKey' not in kwargs:
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
            TrackingGeometryCondAlgCfg)
        cond_alg = TrackingGeometryCondAlgCfg(flags)
        acc.merge(cond_alg)
        kwargs.setdefault("TrackingGeometryReadKey",
                          cond_alg.getPrimary().TrackingGeometryWriteKey)

    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
        InDetExtrapolatorCfg(flags)))

    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasNavigatorCfg, AtlasEnergyLossUpdatorCfg,
        InDetMaterialEffectsUpdatorCfg, InDetMultipleScatteringUpdatorCfg)
    kwargs.setdefault("NavigatorTool", acc.popToolsAndMerge(
        AtlasNavigatorCfg(flags, name="InDetNavigator")))
    kwargs.setdefault("EnergyLossTool", acc.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags)))
    kwargs.setdefault("MaterialUpdateTool", acc.popToolsAndMerge(
        InDetMaterialEffectsUpdatorCfg(flags)))
    kwargs.setdefault("MultipleScatteringTool", acc.popToolsAndMerge(
        InDetMultipleScatteringUpdatorCfg(flags)))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import InDetPropagatorCfg
    kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
        InDetPropagatorCfg(flags)))

    from TrkConfig.TrkMeasurementUpdatorConfig import InDetUpdatorCfg
    kwargs.setdefault("MeasurementUpdateTool", acc.popToolsAndMerge(
        InDetUpdatorCfg(flags)))

    from TrkConfig.TrkResidualPullCalculatorConfig import ResidualPullCalculatorCfg
    kwargs.setdefault("ResidualPullCalculatorTool", acc.popToolsAndMerge(
        ResidualPullCalculatorCfg(flags)))

    kwargs.setdefault("StraightLine", not flags.BField.solenoidOn)
    kwargs.setdefault("OutlierCut", 4)
    kwargs.setdefault("SignedDriftRadius", True)
    kwargs.setdefault("ReintegrateOutliers", True)
    kwargs.setdefault("RecalibrateSilicon", True)
    kwargs.setdefault("RecalibrateTRT", True)
    kwargs.setdefault("TRTTubeHitCut", 1.75)
    kwargs.setdefault("MaxIterations", 40)
    kwargs.setdefault("Acceleration", True)
    kwargs.setdefault("RecalculateDerivatives",
                      flags.Tracking.doMinBias or
                      flags.Beam.Type is BeamType.Cosmics or
                      flags.Tracking.doBeamGas)
    kwargs.setdefault("TRTExtensionCuts", True)
    kwargs.setdefault("TrackChi2PerNDFCut", 7)

    acc.setPrivateTools(CompFactory.Trk.GlobalChi2Fitter(name, **kwargs))
    return acc


def InDetGlobalChi2FitterCfg(flags, name='InDetGlobalChi2Fitter', **kwargs):
    acc = ComponentAccumulator()

    if 'RotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(
            InDetRotCreatorCfg(flags)))
    if 'BroadRotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
            InDetBroadRotCreatorCfg)
        kwargs.setdefault('BroadRotCreatorTool', acc.popToolsAndMerge(
            InDetBroadRotCreatorCfg(flags)))

    # PHF cut during fit iterations to save CPU time
    kwargs.setdefault('MinPHFCut', flags.Tracking.ActiveConfig.minTRTPrecFrac)

    if (flags.Tracking.useBroadPixClusterErrors or
            flags.Tracking.useBroadSCTClusterErrors):
        kwargs.setdefault('RecalibrateSilicon', False)

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault('MaxOutliers', 99)

    if (flags.Beam.Type is BeamType.Cosmics or
            flags.Tracking.doBeamGas):
        kwargs.setdefault('Acceleration', False)

    if flags.Tracking.materialInteractions and not flags.BField.solenoidOn:
        kwargs.setdefault('Momentum', 1000.*Units.MeV)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetGlobalChi2FitterBaseCfg(flags, name, **kwargs)))
    return acc


def InDetGlobalChi2FitterTRTCfg(
        flags, name='InDetGlobalChi2FitterTRT', **kwargs):
    acc = ComponentAccumulator()
    '''
    Global Chi2 Fitter for TRT segments with different settings
    '''

    if 'RotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
            InDetRefitRotCreatorCfg)
        kwargs.setdefault("RotCreatorTool", acc.popToolsAndMerge(
            InDetRefitRotCreatorCfg(flags)))

    kwargs.setdefault("MaterialUpdateTool", '')
    kwargs.setdefault("SignedDriftRadius", True)
    kwargs.setdefault("RecalibrateSilicon", False)
    kwargs.setdefault("RecalibrateTRT", False)
    kwargs.setdefault("TRTTubeHitCut", 2.5)
    kwargs.setdefault("MaxIterations", 10)
    kwargs.setdefault("Acceleration", False)
    kwargs.setdefault("RecalculateDerivatives", False)
    kwargs.setdefault("TRTExtensionCuts", True)
    kwargs.setdefault("TrackChi2PerNDFCut", 999999)

    if flags.Tracking.materialInteractions and not flags.BField.solenoidOn:
        kwargs.setdefault('Momentum', 1000.*Units.MeV)

    kwargs.setdefault("OutlierCut", 5)
    kwargs.setdefault("MaxOutliers",
                      99 if flags.Beam.Type is BeamType.Cosmics else 10)
    kwargs.setdefault("ReintegrateOutliers", False)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetGlobalChi2FitterBaseCfg(flags, name, **kwargs)))
    return acc


def InDetGlobalChi2FitterLowPtCfg(
        flags, name='InDetGlobalChi2FitterLowPt', **kwargs):
    acc = ComponentAccumulator()

    if 'RotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(
            InDetRotCreatorCfg(flags)))
    if 'BroadRotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
            InDetBroadRotCreatorCfg)
        kwargs.setdefault('BroadRotCreatorTool', acc.popToolsAndMerge(
            InDetBroadRotCreatorCfg(flags)))

    kwargs.setdefault('OutlierCut', 5.0)
    kwargs.setdefault('Acceleration', False)
    kwargs.setdefault('RecalculateDerivatives', True)
    kwargs.setdefault('TrackChi2PerNDFCut', 10)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetGlobalChi2FitterBaseCfg(flags, name, **kwargs)))
    return acc


def InDetGlobalChi2FitterBTCfg(
        flags, name='InDetGlobalChi2FitterBT', **kwargs):
    '''
    Global Chi2 Fitter for backtracking
    '''
    kwargs.setdefault("MinPHFCut", 0.)
    return InDetGlobalChi2FitterCfg(flags, name, **kwargs)


##############################
#   Muon configs
##############################
def MuonChi2TrackFitterCfg(flags, name='MuonChi2TrackFitter', **kwargs):
    result = ComponentAccumulator()

    from TrkConfig.TrkRIO_OnTrackCreatorConfig import MuonRotCreatorCfg
    kwargs.setdefault("RotCreatorTool", result.popToolsAndMerge(
        MuonRotCreatorCfg(flags)))

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdatorCfg
    kwargs.setdefault("MeasurementUpdateTool", result.popToolsAndMerge(
        KalmanUpdatorCfg(flags, name='MuonMeasUpdator')))

    if 'ExtrapolationTool' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg
        kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(
            MuonExtrapolatorCfg(flags)))

    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasMultipleScatteringUpdatorCfg)
    kwargs.setdefault("MultipleScatteringTool", result.popToolsAndMerge(
        AtlasMultipleScatteringUpdatorCfg(flags, UseTrkUtils=True)))

    kwargs.setdefault("StraightLine", False)
    kwargs.setdefault("OutlierCut", 3.0)
    kwargs.setdefault("GetMaterialFromTrack", False)
    kwargs.setdefault("RejectLargeNScat", True)

    # take propagator and navigator from the extrapolator
    extrapolator = kwargs['ExtrapolationTool']
    kwargs.setdefault("PropagatorTool",
                      extrapolator.Propagators[0]
                      if len(extrapolator.Propagators) > 0
                      else "")
    kwargs.setdefault("NavigatorTool",  extrapolator.Navigator)
    kwargs.setdefault("EnergyLossTool", extrapolator.EnergyLossUpdater)

    # We need to include the tracking geometry conditions
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
        TrackingGeometryCondAlgCfg)
    geom_cond_key = result.getPrimaryAndMerge(
        TrackingGeometryCondAlgCfg(flags)).TrackingGeometryWriteKey
    kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)

    result.setPrivateTools(CompFactory.Trk.GlobalChi2Fitter(name, **kwargs))
    return result


def MCTBFitterCfg(flags, name='MCTBFitter', **kwargs):
    from TrkConfig.AtlasExtrapolatorConfig import MCTBExtrapolatorCfg
    result = MCTBExtrapolatorCfg(flags)
    kwargs.setdefault("GetMaterialFromTrack", True)
    kwargs.setdefault("ExtrapolationTool", result.popPrivateTools())
    result.setPrivateTools(result.popToolsAndMerge(
        MuonChi2TrackFitterCfg(flags, name, **kwargs)))
    return result


def MCTBFitterMaterialFromTrackCfg(
        flags, name='MCTBFitterMaterialFromTrack', **kwargs):
    return MCTBFitterCfg(flags, name, GetMaterialFromTrack=True, **kwargs)


def MCTBSLFitterCfg(flags, name='MCTBSLFitter', **kwargs):
    kwargs["StraightLine"] = True       # always set
    kwargs.setdefault("Momentum", flags.Muon.straightLineFitMomentum)
    return MCTBFitterCfg(flags, name, **kwargs)


def MCTBSLFitterMaterialFromTrackCfg(
        flags, name='MCTBSLFitterMaterialFromTrack', **kwargs):
    result = ComponentAccumulator()
    kwargs["GetMaterialFromTrack"] = True  # always set
    from TrkConfig.AtlasExtrapolatorConfig import (
        MuonStraightLineExtrapolatorCfg)
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(
        MuonStraightLineExtrapolatorCfg(flags)))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    kwargs["PropagatorTool"] = result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags, name="MuonRK_Propagator"))

    result.setPrivateTools(result.popToolsAndMerge(
        MCTBSLFitterCfg(flags, name, **kwargs)))
    return result


# Unused
def MuonCombinedTrackFitterCfg(
        flags, name="MuonCombinedTrackFitter", **kwargs):
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasNavigatorCfg, AtlasEnergyLossUpdatorCfg)
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import MuonRotCreatorCfg
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        MuonCombinedPropagatorCfg)
    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdatorCfg

    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("ExtrapolationTool", result.popPrivateTools())
    kwargs.setdefault("NavigatorTool", result.popToolsAndMerge(
        AtlasNavigatorCfg(flags)))
    kwargs.setdefault("PropagatorTool", result.popToolsAndMerge(
        MuonCombinedPropagatorCfg(flags)))
    kwargs.setdefault("RotCreatorTool", result.popToolsAndMerge(
        MuonRotCreatorCfg(flags)))
    kwargs.setdefault("EnergyLossTool", result.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags)))
    kwargs.setdefault("MeasurementUpdateTool", result.popToolsAndMerge(
        KalmanUpdatorCfg(flags, name="MuonMeasUpdator")))

    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
        TrackingGeometryCondAlgCfg)
    result.merge(TrackingGeometryCondAlgCfg(flags))
    kwargs.setdefault("TrackingGeometryReadKey", "AtlasTrackingGeometry")

    kwargs.setdefault("ExtrapolatorMaterial", True)

    from MuonCombinedConfig.MuonCombinedRecToolsConfig import (
        MuidMaterialEffectsOnTrackProviderCfg)
    kwargs.setdefault("MuidTool", result.popToolsAndMerge(
        MuidMaterialEffectsOnTrackProviderCfg(flags)))
    kwargs.setdefault("MuidToolParam", None)

    if flags.Beam.Type is BeamType.Cosmics:
        from MuonCombinedConfig.MuonCombinedRecToolsConfig import (
            MuidMaterialEffectsOnTrackProviderParamCfg)
        kwargs.setdefault("MuidToolParam", result.popToolsAndMerge(
            MuidMaterialEffectsOnTrackProviderParamCfg(flags)))

    kwargs.setdefault("MuidMat", True)
    kwargs.setdefault("StraightLine", flags.Beam.Type is BeamType.Cosmics)
    # ^ Was: not jobproperties.BField.solenoidOn() and not jobproperties.BField.allToroidOn()
    kwargs.setdefault("MaxIterations", 50)
    kwargs.setdefault("GetMaterialFromTrack",
                      flags.Beam.Type is not BeamType.Cosmics)
    # ^ Was: jobproperties.BField.solenoidOn() and jobproperties.BField.allToroidOn()
    kwargs.setdefault("RecalculateDerivatives", False)
    kwargs.setdefault("UseCaloTG", True)

    result.setPrivateTools(CompFactory.Trk.GlobalChi2Fitter(name, **kwargs))
    return result


##################################
# InDet trigger config
##################################
def InDetTrigGlobalChi2FitterCfg(flags, name='InDetTrigTrackFitter', **kwargs):
    acc = ComponentAccumulator()

    if 'TrackingGeometryReadKey' not in kwargs:
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
            TrackingGeometryCondAlgCfg)
        cond_alg = TrackingGeometryCondAlgCfg(flags)
        acc.merge(cond_alg)
        kwargs.setdefault("TrackingGeometryReadKey",
                          cond_alg.getPrimary().TrackingGeometryWriteKey)

    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
        InDetExtrapolatorCfg(flags, name="InDetTrigExtrapolator")))
    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasNavigatorCfg, AtlasEnergyLossUpdatorCfg,
        AtlasMaterialEffectsUpdatorCfg)
    kwargs.setdefault("NavigatorTool", acc.popToolsAndMerge(
        AtlasNavigatorCfg(flags, name="InDetTrigNavigator")))
    kwargs.setdefault("EnergyLossTool", acc.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags)))
    kwargs.setdefault("MaterialUpdateTool", acc.popToolsAndMerge(
        AtlasMaterialEffectsUpdatorCfg(flags,
                                       name="InDetTrigMaterialEffectsUpdator")))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags, name="InDetTrigRKPropagator",
                                # revisit after mc23a
                                AccuracyParameter=0.0001,
                                MaxStraightLineStep=0.004)))

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdatorCfg
    kwargs.setdefault("MeasurementUpdateTool", acc.popToolsAndMerge(
        KalmanUpdatorCfg(flags, name="InDetTrigUpdator")))

    from TrkConfig.TrkRIO_OnTrackCreatorConfig import TrigRotCreatorCfg
    kwargs.setdefault("RotCreatorTool", acc.popToolsAndMerge(
        TrigRotCreatorCfg(flags)))
    # InDetTrigBroadInDetRotCreator, #TODO, we have function to configure it
    kwargs.setdefault("BroadRotCreatorTool", None)

    kwargs.setdefault("StraightLine", not flags.BField.solenoidOn)
    kwargs.setdefault("OutlierCut", 4)
    kwargs.setdefault("SignedDriftRadius", True)
    kwargs.setdefault("TrackChi2PerNDFCut", 9)
    kwargs.setdefault("TRTExtensionCuts", True)
    kwargs.setdefault("MaxIterations", 40)
    kwargs.setdefault("Acceleration", True)
    kwargs.setdefault("MaxOutliers", 10)

    acc.setPrivateTools(CompFactory.Trk.GlobalChi2Fitter(name, **kwargs))
    return acc


def InDetTrigGlobalChi2FitterCosmicsCfg(
        flags, name='InDetTrigTrackFitterCosmics', **kwargs):

    kwargs.setdefault("Acceleration", False)
    kwargs.setdefault("OutlierCut", 10)
    kwargs.setdefault("TrackChi2PerNDFCut", 20)
    kwargs.setdefault("MaxOutliers", 99)
    kwargs.setdefault("TRTExtensionCuts", False)

    return InDetTrigGlobalChi2FitterCfg(flags, name, **kwargs)


#########################
# ITk configs
#########################
def ITkGlobalChi2FitterBaseCfg(flags, name='ITkGlobalChi2FitterBase', **kwargs):
    acc = ComponentAccumulator()

    if 'TrackingGeometryReadKey' not in kwargs:
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
            TrackingGeometryCondAlgCfg)
        geom_cond = TrackingGeometryCondAlgCfg(flags)
        acc.merge(geom_cond)
        kwargs.setdefault("TrackingGeometryReadKey",
                          geom_cond.getPrimary().TrackingGeometryWriteKey)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags)))

    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasNavigatorCfg, AtlasEnergyLossUpdatorCfg,
        ITkMaterialEffectsUpdatorCfg, ITkMultipleScatteringUpdatorCfg)
    kwargs.setdefault("NavigatorTool", acc.popToolsAndMerge(
        AtlasNavigatorCfg(flags)))
    kwargs.setdefault("EnergyLossTool", acc.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags)))
    kwargs.setdefault("MaterialUpdateTool", acc.popToolsAndMerge(
        ITkMaterialEffectsUpdatorCfg(flags)))
    kwargs.setdefault("MultipleScatteringTool", acc.popToolsAndMerge(
        ITkMultipleScatteringUpdatorCfg(flags)))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import ITkPropagatorCfg
    kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
        ITkPropagatorCfg(flags)))
    from TrkConfig.TrkMeasurementUpdatorConfig import ITkUpdatorCfg
    kwargs.setdefault("MeasurementUpdateTool", acc.popToolsAndMerge(
        ITkUpdatorCfg(flags)))

    kwargs.setdefault("StraightLine", not flags.BField.solenoidOn)
    kwargs.setdefault("OutlierCut", 4)
    kwargs.setdefault("SignedDriftRadius", True)
    kwargs.setdefault("ReintegrateOutliers", True)
    kwargs.setdefault("RecalibrateSilicon", True)
    kwargs.setdefault("MaxIterations", 40)
    kwargs.setdefault("Acceleration", True)
    kwargs.setdefault("RecalculateDerivatives",
                      flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("TrackChi2PerNDFCut", 7)

    acc.setPrivateTools(CompFactory.Trk.GlobalChi2Fitter(name, **kwargs))
    return acc


def ITkGlobalChi2FitterCfg(flags, name='ITkGlobalChi2Fitter', **kwargs):
    acc = ComponentAccumulator()

    if 'RotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(
            ITkRotCreatorCfg(flags)))

    if 'BroadRotCreatorTool' not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkBroadRotCreatorCfg
        kwargs.setdefault('BroadRotCreatorTool', acc.popToolsAndMerge(
            ITkBroadRotCreatorCfg(flags)))

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault('MaxOutliers', 99)
        kwargs.setdefault('Acceleration', False)

    if flags.Tracking.materialInteractions and not flags.BField.solenoidOn:
        kwargs.setdefault('Momentum', 1000.*Units.MeV)

    acc.setPrivateTools(acc.popToolsAndMerge(
        ITkGlobalChi2FitterBaseCfg(flags, name, **kwargs)))
    return acc
