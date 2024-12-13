#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define methods to configure beam effects with the ComponentAccumulator"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep

# possible components from BeamEffectsConf
# todo names required to copy function name? what are names used for?
# todo add default construction options to make these potentiall useful
# todo verify and add suggestions made in todo


# GenEventManipulators
def ValidityCheckerCfg(flags, name="GenEventValidityChecker", **kwargs):
    """Return a validity checker tool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Simulation.GenEventValidityChecker(name, **kwargs))
    return acc


def GenEventRotatorCfg(flags, name="GenEventRotator", **kwargs):
    """Return a event rotator tool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Simulation.GenEventRotator(name, **kwargs))
    return acc


def GenEventBeamEffectBoosterCfg(flags, name="GenEventBeamEffectBooster", **kwargs):
    """Return a lorentz booster tool"""
    # todo needs random seed, more?
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Simulation.GenEventBeamEffectBooster(name, **kwargs))
    return acc


def GenEventVertexPositionerCfg(flags, name="GenEventVertexPositioner", **kwargs):
    """Return a vertex positioner tool"""
    # todo needs input file(s?)

    acc = ComponentAccumulator()

    from SimulationConfig.SimEnums import VertexSource
    if flags.Sim.VertexSource is VertexSource.MatchingBkg:
        kwargs.setdefault("VertexShifters", [acc.popToolsAndMerge(MatchingBkgVertexPositionerCfg(flags))])
    elif flags.Sim.VertexSource is VertexSource.CondDB:
        kwargs.setdefault("VertexShifters", [acc.popToolsAndMerge(VertexBeamCondPositionerCfg(flags))])
    elif flags.Sim.VertexSource is VertexSource.LongBeamspotVertexPositioner:
        kwargs.setdefault("VertexShifters", [acc.popToolsAndMerge(LongBeamspotVertexPositionerCfg(flags))])

    acc.setPrivateTools(CompFactory.Simulation.GenEventVertexPositioner(name, **kwargs))
    return acc


# LorentzVectorGenerators
def VertexBeamCondPositionerCfg(flags, name="VertexBeamCondPositioner", **kwargs):
    """Return a conditional (? todo) vertex positioner tool"""
    from RngComps.RngCompsConfig import AthRNGSvcCfg

    acc = ComponentAccumulator()

    kwargs.setdefault("RandomSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("SimpleTimeSmearing", flags.Sim.VertexTimeSmearing)
    kwargs.setdefault("TimeWidth", flags.Sim.VertexTimeWidth)

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    acc.setPrivateTools(CompFactory.Simulation.VertexBeamCondPositioner(name, **kwargs))
    return acc


def MatchingBkgVertexPositionerCfg(flags, name="MatchingBkgVertexPositioner", **kwargs):
    """Return a vertex positioner tool that reads a matching vertex from the background input file."""
    acc = ComponentAccumulator()

    if flags.Overlay.DataOverlay:
        kwargs.setdefault("PrimaryVertexContainerName", f"{flags.Overlay.BkgPrefix}PrimaryVertices")

        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'xAOD::VertexContainer#{kwargs["PrimaryVertexContainerName"]}']))

    acc.setPrivateTools(CompFactory.Simulation.MatchingBkgVertexPositioner(name, **kwargs))
    return acc


def CrabKissingVertexPositionerCfg(flags, name="CrabKissingVertexPositioner", **kwargs):
    """Return a Crab-Kissing vertex positioner tool"""
    # todo needs BunchLength, RandomSvc, BunchShape
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Simulation.CrabKissingVertexPositioner(name, **kwargs))
    return acc


def LongBeamspotVertexPositionerCfg(flags, name="LongBeamspotVertexPositioner", **kwargs):
    """Return a long beamspot vertex positioner tool"""
    # todo needs LParameter and RandomSvc
    acc = ComponentAccumulator()
    kwargs.setdefault("SimpleTimeSmearing", flags.Sim.VertexTimeSmearing)
    acc.setPrivateTools(CompFactory.Simulation.LongBeamspotVertexPositioner(name, **kwargs))
    return acc


def BeamEffectsAlgCfg(flags, name="BeamEffectsAlg", **kwargs):
    """Return an accumulator and algorithm for beam effects, wihout output"""
    acc = ComponentAccumulator()

    from SimulationConfig.SimEnums import LArParameterization
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        kwargs.setdefault("ISFRun", True)
    else:
        kwargs.setdefault("ISFRun", flags.Sim.ISFRun)

    # Set default properties
    if flags.Sim.DoFullChain and flags.Digitization.PileUp:
        kwargs.setdefault("InputMcEventCollection", "OriginalEvent_SG+GEN_EVENT")
    else:
        kwargs.setdefault("InputMcEventCollection", "GEN_EVENT")

    if flags.Sim.DoFullChain and flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        kwargs.setdefault('OutputMcEventCollection', f"{flags.Overlay.SigPrefix}TruthEvent")
    else:
        kwargs.setdefault('OutputMcEventCollection', 'BeamTruthEvent')

    # Set (todo) the appropriate manipulator tools
    manipulators = []
    manipulators.append(acc.popToolsAndMerge(ValidityCheckerCfg(flags)))
    from SimulationConfig.SimEnums import VertexSource
    if not flags.Sim.VertexSource == VertexSource.AsGenerated:
        # Vertex manipulation required
        from SimulationConfig.SimEnums import CavernBackground
        if flags.Beam.Type not in [BeamType.Cosmics, BeamType.TestBeam] and flags.Sim.CavernBackground is not CavernBackground.Read:
            manipulators.append(acc.popToolsAndMerge(GenEventVertexPositionerCfg(flags)))
        # manipulators.append(acc.popToolsAndMerge(GenEventBeamEffectBoosterCfg(flags))) # todo segmentation violation
        # manipulators.append(acc.popToolsAndMerge(CrabKissingVertexPositionerCfg(flags))) # todo Callback registration failed
    kwargs.setdefault("GenEventManipulators", manipulators)

    acc.addEventAlgo(CompFactory.Simulation.BeamEffectsAlg(name, **kwargs), primary=True)
    return acc


def BeamEffectsAlgOutputCfg(flags, **kwargs):
    """Return an accumulator and algorithm for beam effects, with output"""
    acc = BeamEffectsAlgCfg(flags, **kwargs)
    # Set to write HITS pool file
    alg = acc.getPrimary()
    ItemList = ["McEventCollection#" + alg.OutputMcEventCollection]
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, "HITS", ItemList=ItemList, disableEventTag=True))
    return acc


def BeamSpotFixerAlgCfg(flags, name="BeamSpotFixerAlg", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)

    kwargs.setdefault("InputKey", "Input_EventInfo")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputKey", flags.Overlay.BkgPrefix + "EventInfo")
    else:
        kwargs.setdefault("OutputKey", "EventInfo")

    acc.addEventAlgo(CompFactory.Simulation.BeamSpotFixerAlg(name, **kwargs))
    return acc


def ZeroLifetimePositionerCfg(flags, name="ZeroLifetimePositioner", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('ApplyPatch', True)
    kwargs.setdefault('RemovePatch', True)
    result.addService(CompFactory.Simulation.ZeroLifetimePositioner(name, **kwargs), primary = True)
    return result


def BeamSpotReweightingAlgCfg(flags, name="BeamSpotReweightingAlg", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc = BeamSpotCondAlgCfg(flags)

    kwargs.setdefault("Input_beam_sigma_z", flags.Digitization.InputBeamSigmaZ)

    acc.addEventAlgo(CompFactory.Simulation.BeamSpotReweightingAlg(name, **kwargs))

    # Ignore dependencies
    from AthenaConfiguration.MainServicesConfig import OutputUsageIgnoreCfg
    acc.merge(OutputUsageIgnoreCfg(flags, name))

    return acc


if __name__ == "__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Set up logging
    log.setLevel(DEBUG)

    import os
    inputDir = os.environ.get("ATLAS_REFERENCE_DATA",
                              "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art")
    # Provide input
    flags.Input.Files = [
        inputDir +
        "/SimCoreTests/e_E50_eta34_49.EVNT.pool.root"
        ]

    # Specify output
    flags.Output.HITSFileName = "myHITS.pool.root"

    # set the source of vertex positioning
    from SimulationConfig.SimEnums import VertexSource
    # flags.Sim.VertexSource = VertexSource.VertexOverrideFile
    flags.Sim.VertexSource = VertexSource.CondDB
    # flags.Sim.VertexSource = VertexSource.LongBeamspotVertexPositioner"

    # included to stop segmentation error - TODO see why it's failing
    flags.Input.isMC = True
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-14"  # conditions tag for conddb (which one to use - old one for simulation)
    flags.Input.RunNumbers = [284500]  # run test job with and without run number and 222510

    # Finalize
    flags.lock()

    # Initialize a new component accumulator
    cfg = MainServicesCfg(flags)  # use this syntax for storegate
    # Add configuration to read EVNT pool file
    cfg.merge(PoolReadCfg(flags))

    # Make use of our defiend function
    cfg.merge(BeamEffectsAlgCfg(flags))

    cfg.getService("StoreGateSvc").Dump = True
    cfg.printConfig(withDetails=True)
    flags.dump()

    # Store in a pickle file
    with open("BeamEffectsAlg.pkl", "wb") as f:
        cfg.store(f)

    # Run it in athena
    cfg.run(maxEvents=20)
