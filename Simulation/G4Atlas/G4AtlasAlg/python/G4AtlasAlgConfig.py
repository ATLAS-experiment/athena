# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from G4AtlasServices.G4AtlasServicesConfig import DetectorGeometrySvcCfg, PhysicsListSvcCfg
from ISF_Services.ISF_ServicesConfig import TruthServiceCfg, InputConverterCfg
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg
from G4AtlasTools.G4AtlasToolsConfig import SensitiveDetectorMasterToolCfg, FastSimulationMasterToolCfg, SimHitContainerListCfg
from G4AtlasServices.G4AtlasUserActionConfig import UserActionSvcCfg
from SimulationConfig.SimulationMetadata import writeSimulationParametersMetadata, readSimulationParameters
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def G4AtlasAlgCfg(flags, name="G4AtlasAlg", **kwargs):
    """Return ComponentAccumulator configured for Atlas G4 simulation, without output"""
    # wihout output
    result = ComponentAccumulator()
    from SimulationConfig.SimEnums import LArParameterization
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        # Add a dummy version of the SimKernel to the CA before adding the properly configured version
        # Presently necessary for FastCaloSim to ensure the proper order of ISF_CollectionMerger
        result.addEventAlgo(CompFactory.G4AtlasAlg(name, **kwargs))
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault( "TruthPreselectionTool", result.popToolsAndMerge(TruthPreselectionToolCfg(flags)) )

    kwargs.setdefault("DetGeoSvc", result.getPrimaryAndMerge(DetectorGeometrySvcCfg(flags)).name)

    kwargs.setdefault("InputTruthCollection", "BeamTruthEvent") #tocheck -are these string inputs?
    kwargs.setdefault("OutputTruthCollection", "TruthEvent")
    ## Killing neutrinos

    ## Don"t drop the GeoModel
    kwargs.setdefault("ReleaseGeoModel", flags.Sim.ReleaseGeoModel)
    from SimulationConfig.SimEnums import LArParameterization
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        kwargs.setdefault("ExtraInputs",
                          {('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager'),
                           ('LArfSamplSym', 'ConditionStore+LArfSamplSym'),
                           ('TileSamplingFraction', 'ConditionStore+TileSamplingFraction')})
    kwargs.setdefault("ExtraOutputs", SimHitContainerListCfg(flags))
    ## Record the particle flux during the simulation
    kwargs.setdefault("RecordFlux", flags.Sim.RecordFlux)

    if flags.Sim.FlagAbortedEvents:
        ## default false
        kwargs.setdefault("FlagAbortedEvents", flags.Sim.FlagAbortedEvents)
        if flags.Sim.FlagAbortedEvents and flags.Sim.KillAbortedEvents:
            print("WARNING When G4AtlasAlg.FlagAbortedEvents is True G4AtlasAlg.KillAbortedEvents should be False. Setting G4AtlasAlg.KillAbortedEvents = False now.")
            kwargs.setdefault("KillAbortedEvents", False)

    ## default true
    kwargs.setdefault("KillAbortedEvents", flags.Sim.KillAbortedEvents)

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("AtRndmGenSvc",
                      result.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    kwargs.setdefault("RandomGenerator", "athena")

    # Multi-threading settinggs
    is_hive = flags.Concurrency.NumThreads > 0
    kwargs.setdefault("MultiThreading", is_hive)
    if is_hive:
        kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

    kwargs.setdefault("TruthRecordService", result.getPrimaryAndMerge(TruthServiceCfg(flags)).name)
    kwargs.setdefault("GeoIDSvc", result.getPrimaryAndMerge(GeoIDSvcCfg(flags)).name)

    #input converter
    kwargs.setdefault("InputConverter", result.getPrimaryAndMerge(InputConverterCfg(flags)).name)
    if flags.Sim.ISF.Simulator.isQuasiStable():
        from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
        kwargs.setdefault("QuasiStablePatcher", result.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)).name )

    #sensitive detector master tool
    kwargs.setdefault("SenDetMasterTool", result.addPublicTool(result.popToolsAndMerge(SensitiveDetectorMasterToolCfg(flags))))

    #fast simulation master tool
    kwargs.setdefault("FastSimMasterTool", result.addPublicTool(result.popToolsAndMerge(FastSimulationMasterToolCfg(flags))))

    #Write MetaData container and make it available to the job
    result.merge(writeSimulationParametersMetadata(flags))
    result.merge(readSimulationParameters(flags))  # for FileMetaData creation

    #User action services (Slow...)
    kwargs.setdefault("UserActionSvc", result.getPrimaryAndMerge(UserActionSvcCfg(flags)).name)

    #PhysicsListSvc
    kwargs.setdefault("PhysicsListSvc", result.getPrimaryAndMerge(PhysicsListSvcCfg(flags)).name)

    ## G4AtlasAlg verbosities (available domains = Navigator, Propagator, Tracking, Stepping, Stacking, Event)
    ## Set stepper verbose = 1 if the Athena logging level is <= DEBUG
    # TODO: Why does it complain that G4AtlasAlgConf.G4AtlasAlg has no "Verbosities" object? Fix.
    # FIXME GaudiConfig2 seems to fail to distinguish an empty dict {} from None
    verbosities=dict(foo="bar")
    kwargs.setdefault("Verbosities", verbosities)

    # Set commands for the G4AtlasAlg
    kwargs.setdefault("G4Commands", flags.Sim.G4Commands)
    from SimulationConfig.SimEnums import CalibrationRun
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        kwargs.setdefault("ExtraOutputs", [( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitActive_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )])
    result.addEventAlgo(CompFactory.G4AtlasAlg(name, **kwargs))

    return result
