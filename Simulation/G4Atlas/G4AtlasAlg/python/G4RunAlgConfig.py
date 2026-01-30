# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from ISF_Services.ISF_ServicesConfig import TruthServiceCfg, InputConverterCfg
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg
from G4AtlasTools.G4AtlasToolsConfig import SensitiveDetectorMasterToolCfg, FastSimulationMasterToolCfg
from G4RunManagement.G4RunManagementToolConfig import G4RunToolCfg
from G4AtlasServices.G4AtlasUserActionConfig import UserActionSvcCfg
from SimulationConfig.SimulationMetadata import writeSimulationParametersMetadata, readSimulationParameters
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def G4RunAlgCfg(flags, name="G4RunAlg", **kwargs):
    """Return ComponentAccumulator configured for Atlas G4 simulation, without output"""
    # wihout output
    result = ComponentAccumulator()
    from SimulationConfig.SimEnums import LArParameterization
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        # Add a dummy version of the SimKernel to the CA before adding the properly configured version
        # Presently necessary for FastCaloSim to ensure the proper order of ISF_CollectionMerger
        result.addEventAlgo(CompFactory.G4RunAlg(name, **kwargs))
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault( "TruthPreselectionTool", result.popToolsAndMerge(TruthPreselectionToolCfg(flags)) )

    kwargs.setdefault("InputTruthCollection", "BeamTruthEvent") #tocheck -are these string inputs?
    kwargs.setdefault("OutputTruthCollection", "TruthEvent")
    ## Killing neutrinos

    ## Don't drop the GeoModel
    kwargs.setdefault("ReleaseGeoModel", flags.Sim.ReleaseGeoModel)

    from G4AtlasTools.G4AtlasToolsConfig import SimHitContainerListCfg, InputContainerListCfg
    kwargs.setdefault("ExtraOutputs", SimHitContainerListCfg(flags) )
    kwargs.setdefault("ExtraInputs" , InputContainerListCfg(flags))

    from SimulationConfig.SimEnums import LArParameterization
    # Configure fast simulation
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        # Set the path to the simplified calorimeter geometry for particle transport if provided
        if flags.Sim.SimplifiedGeoPath:
            kwargs.setdefault('SimplifiedGeoPath', flags.Sim.SimplifiedGeoPath)

    # Set the path to the simplified calorimeter geometry for particle transport if provided
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim and flags.Sim.SimplifiedGeoPath:
        kwargs.setdefault("SimplifiedGeoPath", flags.Sim.SimplifiedGeoPath)

    if flags.Sim.FlagAbortedEvents:
        ## default false
        kwargs.setdefault("FlagAbortedEvents", flags.Sim.FlagAbortedEvents)
        if flags.Sim.FlagAbortedEvents and flags.Sim.KillAbortedEvents:
            print("WARNING When G4RunAlg.FlagAbortedEvents is True G4RunAlg.KillAbortedEvents should be False. Setting G4RunAlg.KillAbortedEvents = False now.")
            kwargs.setdefault("KillAbortedEvents", False)

    ## default true
    kwargs.setdefault("KillAbortedEvents", flags.Sim.KillAbortedEvents)

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("AtRndmGenSvc",
                      result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))

    # Multi-threading settinggs
    is_hive = flags.Concurrency.NumThreads > 0
    if is_hive:
        # not needed with G4 event loop
        # result.merge(G4ThreadPoolSvcCfg(flags))
        kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

    kwargs.setdefault("TruthRecordService", result.getPrimaryAndMerge(TruthServiceCfg(flags)))
    kwargs.setdefault("GeoIDSvc", result.getPrimaryAndMerge(GeoIDSvcCfg(flags)))

    #input converter
    kwargs.setdefault("InputConverter", result.getPrimaryAndMerge(InputConverterCfg(flags)))
    if flags.Sim.ISF.Simulator.isQuasiStable():
        from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
        kwargs.setdefault("QuasiStablePatcher", result.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)) )

    #User action services (Slow...)
    kwargs.setdefault("UserActionSvc", result.getPrimaryAndMerge(UserActionSvcCfg(flags)))

    #sensitive detector master tool
    kwargs.setdefault("SenDetMasterTool", result.addPublicTool(result.popToolsAndMerge(SensitiveDetectorMasterToolCfg(flags))))

    #fast simulation master tool
    kwargs.setdefault("FastSimMasterTool", result.addPublicTool(result.popToolsAndMerge(FastSimulationMasterToolCfg(flags))))

    #Write MetaData container and make it available to the job
    result.merge(writeSimulationParametersMetadata(flags))
    result.merge(readSimulationParameters(flags))  # for FileMetaData creation

    # Configure G4RunTool with all the services that were moved from G4RunAlg
    kwargs.setdefault("G4RunTool", result.addPublicTool(result.popToolsAndMerge(G4RunToolCfg(flags))))

    result.addEventAlgo(CompFactory.G4RunAlg(name, **kwargs))

    return result
