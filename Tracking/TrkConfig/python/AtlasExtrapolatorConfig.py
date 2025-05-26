# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# New configuration for ATLAS extrapolator
# The extrapolator  combines
#
# - The mathematical propagation of track
#    parameters to a destination surface via the configured propagators
#    RungeKuttaPropagator or STEP_propagator.
#    They both perform track parameters propagation through
#    the magnetic field. The STEP_propagator
#    additionally includes material effects.
#
# - The application of material effects:
#   1) Either in certain points/surfaces via the
#   relevant MaterialEffectsUpdators, EnergyLoss and
#   MultipleScattering tools,
#   2) or continuously via the STEP propagator.
#
# - The possible navigation procedure determining the starting
#   and destination volume for the extrapolation and provides
#   the boundary surfaces to be be intersected.
#
# We need/can configure :
# The "Global" Propagagor property (usually Runge Kutta )
# The "Global" Material effects updator property
# The Navigator property
# The STEP_Propagator property (for tracking workloads inside dense volumes)
#
# Aditionally and for consistency/clarity we might want to have
# specific settings for each subdetector volume :
# - ITK/ID,
# - Calo,
# - Muon Spectrometer,
# - BeamPipe,
# - Cavern
# Otherwise the global propagator is used.
#
#
# The extrapolator provides many method and one instance can be used
# in various contexts.
# Broadly we have 2 cases.
# - Inner detector and e/gamma rely on RungeKuttaPropagator.
#   They use Material effects updator(s) with no energy loss.
#   As an example e/gamma for extrapolation to calo uses extrapolateDirectly,
#   so not navigation or material effects.
#
# - Muons (and PFlow) do full tracking (intersections) inside the calo.
#   They rely on the STEP_Propagator  (the intersections code always use it).
#   And Muons prefer to use STEP globally.


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
import TrkConfig.AtlasExtrapolatorToolsConfig as TC
from TrkConfig.TrkExSTEP_PropagatorConfig import AtlasSTEP_PropagatorCfg

# For debugging comparisons with old-style config
use_old_names = False


@AccumulatorCache
def AtlasExtrapolatorCfg(flags, name='AtlasExtrapolator'):
    # Default "ATLAS" with some reasonable defaults
    # Rk for Global/ID STEP to be used for dense volumes.
    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasMultipleScatteringUpdatorCfg)

    result = ComponentAccumulator()

    # propagator defaults
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    AtlasRungeKuttaPropagator = result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags))
    AtlasSTEP_Propagator = result.popToolsAndMerge(
        AtlasSTEP_PropagatorCfg(flags))
    ITkPropagator = None
    if flags.Detector.GeometryITk:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import ITkPropagatorCfg
        ITkPropagator = result.popToolsAndMerge(
            ITkPropagatorCfg(flags))

    AtlasPropagators = []
    AtlasPropagators += [AtlasRungeKuttaPropagator]
    AtlasPropagators += [AtlasSTEP_Propagator]
    if flags.Detector.GeometryITk:
        AtlasPropagators += [ITkPropagator]

    # updator defaults
    AtlasMaterialEffectsUpdator = result.popToolsAndMerge(
        TC.AtlasMaterialEffectsUpdatorCfg(flags))
    AtlasMaterialEffectsUpdatorLandau = result.popToolsAndMerge(
        TC.AtlasMaterialEffectsUpdatorLandauCfg(flags))
    ITkMaterialEffectsUpdator = None
    if flags.Detector.GeometryITk:
        ITkMaterialEffectsUpdator = result.popToolsAndMerge(
            TC.ITkMaterialEffectsUpdatorCfg(flags))

    AtlasUpdators = []
    AtlasUpdators += [AtlasMaterialEffectsUpdator]
    AtlasUpdators += [AtlasMaterialEffectsUpdatorLandau]
    if flags.Detector.GeometryITk:
        AtlasUpdators += [ITkMaterialEffectsUpdator]

    AtlasNavigator = result.popToolsAndMerge(TC.AtlasNavigatorCfg(flags))

    # configure propagators/updators according to geometry signature
    AtlasSubPropagators = []
    AtlasSubPropagators += [AtlasRungeKuttaPropagator.name]  # Global
    if flags.Detector.GeometryITk:
        AtlasSubPropagators += [ITkPropagator.name]  # ITk
    else:
        AtlasSubPropagators += [AtlasRungeKuttaPropagator.name]  # ID
    AtlasSubPropagators += [AtlasSTEP_Propagator.name]  # BeamPipe
    AtlasSubPropagators += [AtlasSTEP_Propagator.name]  # Calo
    AtlasSubPropagators += [AtlasSTEP_Propagator.name]  # MS
    AtlasSubPropagators += [AtlasRungeKuttaPropagator.name]  # Cavern

    AtlasSubUpdators = []
    AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # Global
    if flags.Detector.GeometryITk:
        AtlasSubUpdators += [ITkMaterialEffectsUpdator.name]  # ITk
    else:
        AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # ID
    AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # BeamPipe
    AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # Calo
    AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # MS
    AtlasSubUpdators += [AtlasMaterialEffectsUpdator.name]  # Cavern

    AtlasELossUpdater = result.popToolsAndMerge(
        TC.AtlasEnergyLossUpdatorCfg(flags))
    AtlasEnergyLossUpdater = AtlasELossUpdater

    # call the base class constructor
    Extrapolator = CompFactory.Trk.Extrapolator(
        name,
        Navigator=AtlasNavigator,
        MaterialEffectsUpdators=AtlasUpdators,
        MultipleScatteringUpdater=result.popToolsAndMerge(
            AtlasMultipleScatteringUpdatorCfg(flags, UseTrkUtils=True)),
        STEP_Propagator=AtlasSTEP_Propagator,
        Propagators=AtlasPropagators,
        SubPropagators=AtlasSubPropagators,
        SubMEUpdators=AtlasSubUpdators,
        EnergyLossUpdater=AtlasEnergyLossUpdater
    )

    result.setPrivateTools(Extrapolator)
    return result


@AccumulatorCache
def TrigPFlowExtrapolatorCfg(flags, name='HLT_PFlowExtrapolator'):
    # This is the extrapolator which is used in PFlow
    # reconstruction at the level of HLT. It allows to
    # setup the tolerance which influences speed (but also
    # precision) of the extrapolation.
    result = ComponentAccumulator()

    PFlowExtrapolator = result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags, name))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    RungeKuttaPropagator = result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags))
    from TrkConfig.TrkExSTEP_PropagatorConfig import (
        AtlasSTEP_PropagatorCfg)
    pflowSTEP_Propagator = result.popToolsAndMerge(
        AtlasSTEP_PropagatorCfg(flags,Tolerance=flags.Trigger.Jet.PFlowTolerance))

    pflowPropagators = []
    pflowPropagators += [RungeKuttaPropagator]
    pflowPropagators += [pflowSTEP_Propagator]

    pflowSubPropagators = []
    pflowSubPropagators += [RungeKuttaPropagator.name]  # for Global
    pflowSubPropagators += [RungeKuttaPropagator.name]  # for ID
    pflowSubPropagators += [pflowSTEP_Propagator.name]  # for BeamPipe
    pflowSubPropagators += [pflowSTEP_Propagator.name]  # for Calo
    pflowSubPropagators += [pflowSTEP_Propagator.name]  # for MS
    pflowSubPropagators += [RungeKuttaPropagator.name]  # for cavern

    PFlowExtrapolator.Propagators = pflowPropagators
    PFlowExtrapolator.SubPropagators = pflowSubPropagators
    PFlowExtrapolator.STEP_Propagator = pflowSTEP_Propagator

    result.setPrivateTools(PFlowExtrapolator)
    return result


@AccumulatorCache
def egammaCaloExtrapolatorCfg(flags, name='egammaCaloExtrapolator'):
    # e/gamma mainly uses Extrapolate Directly to a particular
    # surface to the calo. We do not do "tracking"
    # as electrons/photon have showered.
    # The means that in practice only the Global
    # propagator is used with not material effects.
    # Configure everything in any case for clarity/consistency.
    result = ComponentAccumulator()
    egammaExtrapolator = result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags, name))

    egammaPropagators = []
    egammaUpdators = []

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
        RungeKuttaPropagatorCfg)
    RungeKuttaPropagator = result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags))
    MaterialEffectsUpdator = result.popToolsAndMerge(
        TC.AtlasMaterialEffectsUpdatorCfg(flags))

    egammaPropagators += [RungeKuttaPropagator]
    egammaUpdators += [MaterialEffectsUpdator]

    from TrkConfig.TrkExSTEP_PropagatorConfig import (
        AtlasNoMatSTEP_PropagatorCfg)
    NoMatSTEP_Propagator = result.popToolsAndMerge(
        AtlasNoMatSTEP_PropagatorCfg(flags))
    NoElossMaterialEffectsUpdator = result.popToolsAndMerge(
        TC.AtlasNoElossMaterialEffectsUpdatorCfg(flags))

    egammaPropagators += [NoMatSTEP_Propagator]
    egammaUpdators += [NoElossMaterialEffectsUpdator]

    if flags.Detector.GeometryITk:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import ITkPropagatorCfg
        ITkPropagator = result.popToolsAndMerge(
            ITkPropagatorCfg(flags))
        ITkMaterialEffectsUpdator = result.popToolsAndMerge(
            TC.ITkMaterialEffectsUpdatorCfg(flags))

        egammaPropagators += [ITkPropagator]
        egammaUpdators += [ITkMaterialEffectsUpdator]

    # configure propagators/updators according to geometry signature
    egammaSubPropagators = []
    egammaSubUpdators = []
    # Global
    egammaSubPropagators += [RungeKuttaPropagator.name]
    egammaSubUpdators += [MaterialEffectsUpdator.name]
    # ID/ITK
    if flags.Detector.GeometryITk:
        egammaSubPropagators += [ITkPropagator.name]
        egammaSubUpdators += [ITkMaterialEffectsUpdator.name]
    else:
        egammaSubPropagators += [RungeKuttaPropagator.name]
        egammaSubUpdators += [MaterialEffectsUpdator.name]
    # BeamPipe
    egammaSubPropagators += [RungeKuttaPropagator.name]
    egammaSubUpdators += [MaterialEffectsUpdator.name]
    # Calo
    egammaSubPropagators += [RungeKuttaPropagator.name]
    egammaSubUpdators += [NoElossMaterialEffectsUpdator.name]
    # MS
    egammaSubPropagators += [NoMatSTEP_Propagator.name]
    egammaSubUpdators += [NoElossMaterialEffectsUpdator.name]
    # Cavern
    egammaSubPropagators += [RungeKuttaPropagator.name]
    egammaSubUpdators += [MaterialEffectsUpdator.name]

    egammaExtrapolator.MaterialEffectsUpdators = egammaUpdators
    egammaExtrapolator.SubMEUpdators = egammaSubUpdators
    egammaExtrapolator.Propagators = egammaPropagators
    egammaExtrapolator.SubPropagators = egammaSubPropagators
    # egamma STEP with no eloss for calo intersections
    egammaExtrapolator.STEP_Propagator = NoMatSTEP_Propagator

    result.setPrivateTools(egammaExtrapolator)
    return result


@AccumulatorCache
def MCTruthClassifierExtrapolatorCfg(flags,
                                     name='MCTruthClassifierExtrapolator'):
    # MCTruthClassifier. Used to Extrapolate Directly
    # photons (fwd Electrons) to a specific surface to the calo
    # for truth matching.
    # We do not do "tracking" as electrons/photon have showered.
    # This means that in practice only the 'Global'
    # propagator and no material effects.
    # Configure everything in any case for clarity/consistency.
    result = ComponentAccumulator()

    MCTruthExtrapolator = result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags, name))

    MCTruthUpdators = []

    NoElossMaterialEffectsUpdator = result.popToolsAndMerge(
        TC.AtlasNoElossMaterialEffectsUpdatorCfg(flags))
    MCTruthUpdators += [NoElossMaterialEffectsUpdator]

    MCTruthSubUpdators = []

    # configure propagators/updators according to geometry signature
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # Global
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # ID
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # beampipe
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # calo
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # MS
    MCTruthSubUpdators += [NoElossMaterialEffectsUpdator.name]  # cavern

    MCTruthExtrapolator.MaterialEffectsUpdators = MCTruthUpdators
    MCTruthExtrapolator.SubMEUpdators = MCTruthSubUpdators

    result.setPrivateTools(MCTruthExtrapolator)
    return result


@AccumulatorCache
def InDetExtrapolatorCfg(flags, name='InDetExtrapolator', **kwargs):
    # Inner detector config cares mainly for the "Global".
    # This is usually the RungeKutta Propagator and a material
    # effects updator without energy loss.
    # As extrapolations are to happen inside the InDet volume.
    result = ComponentAccumulator()

    # FIXME copied from the old config, also needs fixing on the c++ side.
    if 'Propagators' not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
            InDetPropagatorCfg)
        InDetPropagator = result.popToolsAndMerge(
            InDetPropagatorCfg(flags))
        Propagators = [InDetPropagator]
        kwargs.setdefault("Propagators", Propagators)

    propagator = kwargs.get('Propagators')[0].name if (
        kwargs.get('Propagators', None) is not None
        and len(kwargs.get('Propagators', None)) > 0) else None

    if 'MaterialEffectsUpdators' not in kwargs:
        InDetMaterialEffectsUpdator = result.getPrimaryAndMerge(
            TC.InDetMaterialEffectsUpdatorCfg(flags))
        MaterialEffectsUpdators = [InDetMaterialEffectsUpdator]
        kwargs.setdefault("MaterialEffectsUpdators", MaterialEffectsUpdators)
    material_updator = kwargs.get('MaterialEffectsUpdators')[0].name if (
        kwargs.get('MaterialEffectsUpdators', None) is not None
        and len(kwargs.get('MaterialEffectsUpdators', None)) > 0) else None

    if 'Navigator' not in kwargs:
        AtlasNavigator = result.popToolsAndMerge(
            TC.AtlasNavigatorCfg(flags, name="InDetNavigator"))
        kwargs.setdefault("Navigator", AtlasNavigator)

    sub_propagators = []
    sub_updators = []

    # configure propagators/updators according to geometry signature
    # Global entry
    sub_propagators += [propagator]
    sub_updators += [material_updator]

    # ID entry
    sub_propagators += [propagator]
    sub_updators += [material_updator]

    # beampipe entry
    sub_updators += [material_updator]

    kwargs.setdefault("SubPropagators", sub_propagators)
    kwargs.setdefault("SubMEUpdators", sub_updators)

    AtlasELossUpdater = result.popToolsAndMerge(
        TC.AtlasEnergyLossUpdatorCfg(flags))
    kwargs.setdefault("EnergyLossUpdater", AtlasELossUpdater)

    extrapolator = CompFactory.Trk.Extrapolator(name, **kwargs)
    result.setPrivateTools(extrapolator)
    return result


def MuonExtrapolatorCfg(flags, name="MuonExtrapolator", **kwargs):
    from TrkConfig.AtlasExtrapolatorToolsConfig import (
        AtlasMultipleScatteringUpdatorCfg)

    # Muons set the STEP also as the single "Global" propagator
    result = ComponentAccumulator()

    AtlasMaterialEffectsUpdator = result.popToolsAndMerge(
        TC.AtlasMaterialEffectsUpdatorCfg(flags,
                                          name="MuonMaterialEffectsUpdator"))
    kwargs.setdefault("MaterialEffectsUpdators", [AtlasMaterialEffectsUpdator])

    kwargs.setdefault("MultipleScatteringUpdater",
                      result.popToolsAndMerge(
                          AtlasMultipleScatteringUpdatorCfg(flags,
                                                            UseTrkUtils=True)))

    AtlasNavigator = result.popToolsAndMerge(TC.AtlasNavigatorCfg(
        flags, name='InDetNavigator' if use_old_names else None))
    kwargs.setdefault("Navigator", AtlasNavigator)

    AtlasELossUpdater = result.popToolsAndMerge(
        TC.AtlasEnergyLossUpdatorCfg(flags))
    kwargs.setdefault("EnergyLossUpdater", AtlasELossUpdater)

    if 'STEP_Propagator' not in kwargs:
        AtlasSTEP_Propagator = result.popToolsAndMerge(
            AtlasSTEP_PropagatorCfg(flags))
        kwargs.setdefault("STEP_Propagator", AtlasSTEP_Propagator)

    if 'Propagators' not in kwargs:
        if use_old_names:
            kwargs.setdefault("Propagators", [result.popToolsAndMerge(
                AtlasSTEP_PropagatorCfg(flags, name="MuonPropagator"))])
        else:
            kwargs.setdefault("Propagators", [kwargs["STEP_Propagator"]])

    kwargs.setdefault("ResolveMuonStation", True)
    # must be > 1um to avoid missing MTG intersections
    kwargs.setdefault("Tolerance", 0.0011)

    extrap = CompFactory.Trk.Extrapolator(name, **kwargs)
    result.setPrivateTools(extrap)
    return result


def MuonStraightLineExtrapolatorCfg(flags,
                                    name="MuonStraightLineExtrapolator",
                                    **kwargs):
    # Muon set the STEP also as the single "Global" propagator
    result = ComponentAccumulator()

    muon_prop = None
    if use_old_names:
        muon_prop = result.popToolsAndMerge(
            AtlasSTEP_PropagatorCfg(flags, name='MuonStraightLinePropagator'))
    else:
        muon_prop = result.popToolsAndMerge(AtlasSTEP_PropagatorCfg(flags))
    kwargs.setdefault("Propagators", [muon_prop])
    kwargs.setdefault("STEP_Propagator", muon_prop)
    extrap = result.popToolsAndMerge(
        MuonExtrapolatorCfg(flags, name, **kwargs))
    result.setPrivateTools(extrap)
    result.addPublicTool(extrap)
    # This should be done by the client with the public tool,
    # but since it's hard to track down
    # (and since Extrapolators are a specicial case),
    # just be pragmatic for now.
    return result


def MCTBExtrapolatorCfg(flags, name='MCTBExtrapolator', **kwargs):
    # Muon set the STEP also as the single "Global" propagator
    result = ComponentAccumulator()
    if use_old_names:
        kwargs.setdefault("Propagators", [result.popToolsAndMerge(
            AtlasSTEP_PropagatorCfg(flags, name='MCTBPropagator'))])
        kwargs.setdefault("STEP_Propagator",
                          result.popToolsAndMerge(AtlasSTEP_PropagatorCfg(flags)))
    else:
        prop = result.popToolsAndMerge(
            AtlasSTEP_PropagatorCfg(flags))
        kwargs.setdefault("Propagators", [prop])
        kwargs.setdefault("STEP_Propagator", prop)
    kwargs.setdefault("ResolveMuonStation", False)
    kwargs.setdefault("Navigator", result.popToolsAndMerge(
        TC.AtlasNavigatorCfg(flags, name="InDetNavigator")))
    kwargs.setdefault("EnergyLossUpdater", result.popToolsAndMerge(
        TC.AtlasEnergyLossUpdatorCfg(flags)))

    result.setPrivateTools(result.popToolsAndMerge(
        MuonExtrapolatorCfg(flags, name, **kwargs)))
    return result


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import logging

    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("AtlasExtrapolatorConfigTest")
    mlog.info("Configuring AtlasExtrapolator : ")
    printProperties(mlog, cfg.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags)),
        nestLevel=1,
        printDefaults=True)
    mlog.info("Configuring egammaCaloExtrapolator : ")
    printProperties(mlog, cfg.popToolsAndMerge(
        egammaCaloExtrapolatorCfg(flags)),
        nestLevel=1,
        printDefaults=True)
    mlog.info("Configuring MCTruthClassifierExtrapolator : ")
    printProperties(mlog, cfg.popToolsAndMerge(
        MCTruthClassifierExtrapolatorCfg(flags)),
        nestLevel=1,
        printDefaults=True)

    f = open("atlasextrapolator.pkl", "wb")
    cfg.store(f)
    f.close()
