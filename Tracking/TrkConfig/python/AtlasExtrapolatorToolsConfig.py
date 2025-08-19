# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of tools needed by the Extrapolator
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


# Configured Energy Loss  uses the TrkUtils parametrization for
# energy loss due to ionization and radiation
# Ionization (PDG 2011 formula 27.11)
# H. Bichsel, Rev. Mod. Phys. 60, 663 (1988)
def AtlasEnergyLossUpdatorCfg(flags,
                              name='AtlasEnergyLossUpdator',
                              **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.Trk.EnergyLossUpdator(name, **kwargs))
    return result


# Configured Updators for Layer based material effects
# by default applies Multiple Scattering and
# Energy Loss.
def AtlasMaterialEffectsUpdatorCfg(flags,
                                   name='AtlasMaterialEffectsUpdator',
                                   **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("EnergyLossUpdator", result.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags)))
    kwargs.setdefault("MultipleScatteringUpdator", result.popToolsAndMerge(
        AtlasMultipleScatteringUpdatorCfg(flags)))

    result.setPrivateTools(
        CompFactory.Trk.MaterialEffectsUpdator(name, **kwargs))
    return result


def AtlasMaterialEffectsUpdatorLandauCfg(flags,
                                         name='AtlasMaterialEffectsUpdatorLandau',
                                         **kwargs):
    # Landau most probable energy loss is not additive
    # special treatment on how we accumulate
    kwargs.setdefault("LandauMode", True)
    return AtlasMaterialEffectsUpdatorCfg(flags, name, **kwargs)


def AtlasNoElossMaterialEffectsUpdatorCfg(flags,
                                          name='AtlasNoElossMaterialEffectsUpdator',
                                          **kwargs):
    kwargs.setdefault("EnergyLoss", False)
    return AtlasMaterialEffectsUpdatorCfg(flags, name, **kwargs)


def InDetMaterialEffectsUpdatorCfg(flags,
                                   name="InDetMaterialEffectsUpdator",
                                   **kwargs):
    if not flags.BField.solenoidOn:
        import AthenaCommon.SystemOfUnits as Units
        kwargs.setdefault("EnergyLoss", False)
        kwargs.setdefault("ForceMomentum", True)
        kwargs.setdefault("ForcedMomentumValue", 1000*Units.MeV)
    return AtlasMaterialEffectsUpdatorCfg(flags, name, **kwargs)


def ITkMaterialEffectsUpdatorCfg(flags,
                                 name="ITkMaterialEffectsUpdator",
                                 **kwargs):
    return InDetMaterialEffectsUpdatorCfg(flags, name, **kwargs)


# Navigator used in order to  handle the
# navigation inside the Tracking Geometry
# i.e search for the next TrackingVolume
@AccumulatorCache
def AtlasNavigatorCfg(flags,
                      name='AtlasNavigator',
                      **kwargs):
    # get the correct TrackingGeometry setup
    result = ComponentAccumulator()
    if 'TrackingGeometryKey' not in kwargs:
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
            TrackingGeometryCondAlgCfg)
        acc = TrackingGeometryCondAlgCfg(flags)
        geom_cond_key = acc.getPrimary().TrackingGeometryWriteKey
        result.merge(acc)
        kwargs.setdefault("TrackingGeometryKey", geom_cond_key)
    result.setPrivateTools(CompFactory.Trk.Navigator(name, **kwargs))
    return result


# Temporary until condition algorithm for tracking geometry
# can be used also for FastSim
def FastSimNavigatorCfg(flags,
                        name="AtlasNavigator",
                        **kwargs):
    if flags.Sim.ISF.UseTrackingGeometryCond:
        return AtlasNavigatorCfg(flags, name, **kwargs)

    result = ComponentAccumulator()
    if 'TrackingGeometrySvc' not in kwargs:
        from TrkConfig.AtlasTrackingGeometrySvcConfig import (
            TrackingGeometrySvcCfg)
        acc = TrackingGeometrySvcCfg(flags)
        kwargs.setdefault("TrackingGeometrySvc", acc.getPrimary())
        kwargs.setdefault("TrackingGeometryKey", '')
        result.merge(acc)

    result.setPrivateTools(CompFactory.Trk.Navigator(name, **kwargs))
    return result

# Uses Highland formula for sigma
# from Trk::MaterialInteraction::sigmaMS
# (PDG 2018 formula 33.15)
# G.R. Lynch and O.I Dahl, Nucl. Instrum. Methods B58, 6 (1991).
# Eq. (33.15) is Eq. 12 from this paper
# = default config


def AtlasMultipleScatteringUpdatorCfg(flags,
                                      name="AtlasMultipleScatteringUpdator",
                                      **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseTrkUtils", True)
    acc.setPrivateTools(
        CompFactory.Trk.MultipleScatteringUpdator(name, **kwargs))
    return acc

# Uses Highland formula for sigma
# from Trk::MultipleScatteringUpdator::sigmaSquare
# (PDG 2010 formula 27.14)
# V.L. Highland, Nucl. Instrum. Methods 129, 497 (1975),
# and Nucl. Instrum. Methods 161, 171 (1979)
# = non default config, used for GlobalChi2Fitter


def InDetMultipleScatteringUpdatorCfg(flags,
                                      name="InDetMultipleScatteringUpdator",
                                      **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseTrkUtils", False)
    acc.setPrivateTools(
        CompFactory.Trk.MultipleScatteringUpdator(name, **kwargs))
    return acc


def ITkMultipleScatteringUpdatorCfg(flags,
                                    name="ITkMultipleScatteringUpdator",
                                    **kwargs):
    return InDetMultipleScatteringUpdatorCfg(flags, name, **kwargs)


def fatrasMultipleScatteringUpdatorCfg(flags,
                                       name="ISF_FatrasMultipleScatteringUpdator",
                                       **kwargs):
    result = ComponentAccumulator()

    from ISF_FatrasServices.ISF_FatrasConfig import TrkExRndSvcMTCfg
    kwargs.setdefault("RandomNumberService",
                      result.getPrimaryAndMerge(TrkExRndSvcMTCfg(flags)))
    kwargs.setdefault("RandomStreamName",
                      flags.Sim.Fatras.TrkExRandomStreamName)
    kwargs.setdefault("GaussianMixtureModel",
                      flags.Sim.Fatras.GaussianMixtureModel)

    result.setPrivateTools(
        CompFactory.Trk.MultipleScatteringUpdator(name, **kwargs))
    return result


def NIMatEffUpdatorCfg(flags, name="NIMatEffUpdator", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("MaterialEffectsUpdator", result.popToolsAndMerge(
        AtlasMaterialEffectsUpdatorCfg(flags)))

    result.setPrivateTools(CompFactory.Trk.NIMatEffUpdator(name, **kwargs))
    return result
