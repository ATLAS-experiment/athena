"""Main ISF tools configuration with ComponentAccumulator

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SimulationConfig.SimulationMetadata import writeSimulationParametersMetadata, readSimulationParameters
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg
from ISF_Services.ISF_ServicesConfig import (
    InputConverterCfg, TruthServiceCfg,
    ATLFAST_ParticleBrokerSvcCfg, ParticleBrokerSvcNoOrderingCfg
)
from ISF_Tools.ISF_ToolsConfig import (
    ParticleKillerToolCfg, EnergyParticleOrderingToolCfg,
    ParticleOrderingToolCfg, MemoryMonitorToolCfg
)
from ISF_SimulationSelectors.ISF_SimulationSelectorsConfig import (
    DefaultATLFAST_Geant4SelectorCfg,
    DefaultParticleKillerSelectorCfg,
    EtaGreater5ParticleKillerSimSelectorCfg,
    FullGeant4SelectorCfg,
    MuonATLFAST_Geant4SelectorCfg,
    PassBackGeant4SelectorCfg,
    DefaultFastCaloSimV2SelectorCfg,
    PionATLFAST_Geant4SelectorCfg,
    ProtonATLFAST_Geant4SelectorCfg,
    NeutronATLFAST_Geant4SelectorCfg,
    ChargedKaonATLFAST_Geant4SelectorCfg,
    KLongATLFAST_Geant4SelectorCfg,
    DefaultFatrasSelectorCfg,
    DefaultActsSelectorCfg
)
from ISF_Geant4Tools.ISF_Geant4ToolsConfig import (
    ATLFAST_Geant4ToolCfg,
    FullGeant4ToolCfg,
    PassBackGeant4ToolCfg,
)
from ISF_Geant4CommonTools.ISF_Geant4CommonToolsConfig import (
    EntryLayerToolMTCfg,
    ATLFAST_EntryLayerToolMTCfg
)
AthSequencer=CompFactory.AthSequencer

# MT
def Kernel_GenericSimulatorMTCfg(flags, name="ISF_Kernel_GenericSimulatorMT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault("TruthPreselectionTool", acc.popToolsAndMerge(TruthPreselectionToolCfg(flags)))

    if "GeoIDSvc" not in kwargs:
        kwargs.setdefault("GeoIDSvc", acc.getPrimaryAndMerge(GeoIDSvcCfg(flags)).name)

    if "TruthRecordService" not in kwargs:
        kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    if "EntryLayerTool" not in kwargs:
        kwargs.setdefault("EntryLayerTool", acc.addPublicTool(acc.popToolsAndMerge(EntryLayerToolMTCfg(flags))))

    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep == ProductionStep.FastChain:
        if flags.Digitization.PileUp:
            OEsvc = CompFactory.StoreGateSvc("OriginalEvent_SG")
            acc.addService(OEsvc)
            kwargs.setdefault("EvtStore", OEsvc.name) # TODO check this is correct

    kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)
    kwargs.setdefault("InputEvgenCollection", "BeamTruthEvent")
    kwargs.setdefault("OutputTruthCollection", "TruthEvent")
    from SimulationConfig.SimEnums import CalibrationRun
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        kwargs.setdefault("ExtraOutputs", [
            ( 'CaloCalibrationHitContainer', 'StoreGateSvc+LArCalibrationHitActive_DEAD' ),
            ( 'CaloCalibrationHitContainer', 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ),
            ( 'CaloCalibrationHitContainer', 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )
        ])

    if flags.Sim.ISF.Simulator.isQuasiStable():
        if "QuasiStablePatcher" not in kwargs:
            from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
            kwargs.setdefault("QuasiStablePatcher", acc.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)))
    if "InputConverter" not in kwargs:
        kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(InputConverterCfg(flags)).name)

    if flags.Sim.ISF.ReSimulation:
        acc.addSequence(AthSequencer('SimSequence'), parentName='AthAlgSeq') # TODO make the name configurable?
        acc.addEventAlgo(CompFactory.ISF.SimKernelMT(name, **kwargs), 'SimSequence') # TODO make the name configurable?
    else:
        acc.addEventAlgo(CompFactory.ISF.SimKernelMT(name, **kwargs))
    return acc


def Kernel_GenericSimulatorNoG4MTCfg(flags, name="ISF_Kernel_GenericSimulatorNoG4MT", **kwargs):
    return Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)


def Kernel_GenericG4OnlyMTCfg(flags, name="ISF_Kernel_GenericG4OnlyMT", **kwargs):
    acc = ComponentAccumulator()

    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "CaloSimulationSelectors", "MSSimulationSelectors"])
    if flags.Detector.GeometryCavern:
        # If we are simulating the cavern then we want to use the FullGeant4Selector here too
        defaultG4SelectorRegions.add("CavernSimulationSelectors")
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        pubTool = acc.addPublicTool(acc.popToolsAndMerge(FullGeant4SelectorCfg(flags)))
        # SimulationSelectors are still public ToolHandleArrays currently
        for selectorRegion in defaultG4SelectorRegions:
            kwargs.setdefault(selectorRegion, [pubTool])

    if "CavernSimulationSelectors" not in kwargs:
        kwargs.setdefault("CavernSimulationSelectors",
                          [acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))])

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_FullG4MTCfg(flags, name="ISF_Kernel_FullG4MT", **kwargs):
    acc = ComponentAccumulator()

    if "SimulationTools" not in kwargs:
        kwargs.setdefault("SimulationTools", [
            acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
            acc.popToolsAndMerge(FullGeant4ToolCfg(flags))
        ]) #private ToolHandleArray

    acc.merge(Kernel_GenericG4OnlyMTCfg(flags, name, **kwargs))
    return acc


def Kernel_FullG4MT_QSCfg(flags, name="ISF_Kernel_FullG4MT_QS", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(FullGeant4ToolCfg(flags))
    ]) #private ToolHandleArray

    acc.merge(Kernel_GenericG4OnlyMTCfg(flags, name, **kwargs))
    return acc


def Kernel_PassBackG4MTCfg(flags, name="ISF_Kernel_PassBackG4MT", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "CaloSimulationSelectors", "MSSimulationSelectors"])
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        passBackG4Selector = acc.addPublicTool(acc.popToolsAndMerge(PassBackGeant4SelectorCfg(flags)))
        kwargs.setdefault("BeamPipeSimulationSelectors", [passBackG4Selector])
        kwargs.setdefault("IDSimulationSelectors", [passBackG4Selector])
        kwargs.setdefault("CaloSimulationSelectors", [passBackG4Selector])
        kwargs.setdefault("MSSimulationSelectors", [passBackG4Selector])

    if "CavernSimulationSelectors" not in kwargs:
        kwargs.setdefault("CavernSimulationSelectors",
                          [acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))])

    if "SimulationTools" not in kwargs:
        kwargs.setdefault("SimulationTools", [
            acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
            acc.popToolsAndMerge(PassBackGeant4ToolCfg(flags))
        ])

    if "ParticleOrderingTool" not in kwargs:
        kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(EnergyParticleOrderingToolCfg(flags)))

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFAST3MTCfg(flags, name="ISF_Kernel_ATLFAST3MT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(ParticleOrderingToolCfg(flags)))

    kwargs.setdefault("EntryLayerTool", acc.addPublicTool(acc.popToolsAndMerge(ATLFAST_EntryLayerToolMTCfg(flags)))) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    # BeamPipe, ID, MS Simulation Selectors
    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "MSSimulationSelectors"])
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        pubTool = acc.addPublicTool(acc.popToolsAndMerge(DefaultATLFAST_Geant4SelectorCfg(flags)))
        kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
        kwargs.setdefault("IDSimulationSelectors", [pubTool])
        kwargs.setdefault("MSSimulationSelectors", [pubTool])

    # CaloSimulationSelectors
    kwargs.setdefault("CaloSimulationSelectors", [
        acc.addPublicTool(acc.popToolsAndMerge(MuonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    ])

    # CavernSimulationSelectors
    kwargs.setdefault("CavernSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))) ])

    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ToolCfg
    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags)),
        acc.popToolsAndMerge(ATLFAST_Geant4ToolCfg(flags))
    ])

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_ATLFAST3MT_QSCfg(flags, name="ISF_Kernel_ATLFAST3MT_QS", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(ParticleOrderingToolCfg(flags)))

    kwargs.setdefault("EntryLayerTool", acc.addPublicTool(acc.popToolsAndMerge(ATLFAST_EntryLayerToolMTCfg(flags)))) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    # BeamPipe, ID, MS Simulation Selectors
    defaultG4SelectorRegions = set(["BeamPipeSimulationSelectors", "IDSimulationSelectors", "MSSimulationSelectors"])
    if defaultG4SelectorRegions - kwargs.keys(): # i.e. if any of these have not been defined yet
        pubTool = acc.addPublicTool(acc.popToolsAndMerge(DefaultATLFAST_Geant4SelectorCfg(flags)))
        kwargs.setdefault("BeamPipeSimulationSelectors", [pubTool])
        kwargs.setdefault("IDSimulationSelectors", [pubTool])
        kwargs.setdefault("MSSimulationSelectors", [pubTool])

    # CaloSimulationSelectors
    kwargs.setdefault("CaloSimulationSelectors", [
        acc.addPublicTool(acc.popToolsAndMerge(MuonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    ])

    # CavernSimulationSelectors
    kwargs.setdefault("CavernSimulationSelectors",
                      [ acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))) ])

    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ToolCfg
    kwargs.setdefault("SimulationTools", [ acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
                                                       acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags)),
                                                       acc.popToolsAndMerge(ATLFAST_Geant4ToolCfg(flags)) ])
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_GenericSimulatorCfg(flags, name="ISF_Kernel_GenericSimulator", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    if flags.Sim.UseShadowEvent and "TruthPreselectionTool" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import TruthPreselectionToolCfg
        kwargs.setdefault( "TruthPreselectionTool", acc.popToolsAndMerge(TruthPreselectionToolCfg(flags)) )

    if "TruthRecordService" not in kwargs:
        kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)

    if "MemoryMonitoringTool" not in kwargs:
        kwargs.setdefault("MemoryMonitoringTool", acc.addPublicTool(acc.popToolsAndMerge(MemoryMonitorToolCfg(flags))))

    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", acc.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)).name)

    if flags.Sim.ISF.Simulator.isQuasiStable():
        if "QuasiStablePatcher" not in kwargs:
            from BeamEffects.BeamEffectsAlgConfig import ZeroLifetimePositionerCfg
            kwargs.setdefault("QuasiStablePatcher", acc.getPrimaryAndMerge(ZeroLifetimePositionerCfg(flags)) )
    if "InputConverter" not in kwargs:
        kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(InputConverterCfg(flags)).name)

    kwargs.setdefault("InputHardScatterCollection", "BeamTruthEvent")
    kwargs.setdefault("OutputHardScatterTruthCollection", "TruthEvent")
    kwargs.setdefault("DoCPUMonitoring", flags.Sim.ISF.DoTimeMonitoring)
    kwargs.setdefault("DoMemoryMonitoring", flags.Sim.ISF.DoMemoryMonitoring)

    if flags.Sim.ISF.ReSimulation:
        acc.addSequence(AthSequencer('SimSequence'), parentName='AthAlgSeq') # TODO make the name configurable?
        acc.addEventAlgo(CompFactory.ISF.SimKernel(name, **kwargs), 'SimSequence') # TODO make the name configurable?
    else:
        acc.addEventAlgo(CompFactory.ISF.SimKernel(name, **kwargs))
    return acc


def Kernel_ATLFAST3F_G4MSCfg(flags, name="ISF_Kernel_ATLFAST3F_G4MS", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Force the SimKernel to be before the CollectionMerger by adding it here

    kwargs.setdefault("BeamPipeSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))) ])
    kwargs.setdefault("IDSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultFatrasSelectorCfg(flags))) ])
    kwargs.setdefault("CaloSimulationSelectors", [
        acc.addPublicTool(acc.popToolsAndMerge(MuonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    ])
    kwargs.setdefault("MSSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultATLFAST_Geant4SelectorCfg(flags))) ])
    kwargs.setdefault("CavernSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags))) ])
    #simFlags.SimulationFlavour = "ATLFAST3F_G4MS" # not migrated

    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Merge properly configured SimKernel here and let deduplication sort it out.
    return acc


def Kernel_ATLFAST3F_ACTSMTCfg(flags, name="ISF_Kernel_ATLFAST3F_ACTSMT", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool", acc.popToolsAndMerge(ParticleOrderingToolCfg(flags)))

    kwargs.setdefault("EntryLayerTool", acc.addPublicTool(acc.popToolsAndMerge(ATLFAST_EntryLayerToolMTCfg(flags)))) # public ToolHandle
    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs)) # Workaround

    # BeamPipeSimulationSelectors
    defPartKillerSelector = acc.addPublicTool(acc.popToolsAndMerge(DefaultParticleKillerSelectorCfg(flags)))
    kwargs.setdefault("BeamPipeSimulationSelectors", [ defPartKillerSelector ])
    # IDSimulationSelectors
    kwargs.setdefault("IDSimulationSelectors", [
        acc.addPublicTool(acc.popToolsAndMerge(DefaultActsSelectorCfg(flags))),
        defPartKillerSelector
    ])
    # CaloSimulationSelectors
    kwargs.setdefault("CaloSimulationSelectors", [
        acc.addPublicTool(acc.popToolsAndMerge(MuonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(EtaGreater5ParticleKillerSimSelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(PionATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ProtonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(NeutronATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(ChargedKaonATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(KLongATLFAST_Geant4SelectorCfg(flags))),
        acc.addPublicTool(acc.popToolsAndMerge(DefaultFastCaloSimV2SelectorCfg(flags)))
    ])
    # MSSimulationSelectors
    kwargs.setdefault("MSSimulationSelectors", [ acc.addPublicTool(acc.popToolsAndMerge(DefaultATLFAST_Geant4SelectorCfg(flags))) ])
    # CavernSimulationSelectors
    kwargs.setdefault("CavernSimulationSelectors", [ defPartKillerSelector ])

    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ToolCfg
    from ISF_ActsTools.ISF_ActsToolsConfig import ActsFatrasSimToolCfg
    kwargs.setdefault("SimulationTools", [
        acc.popToolsAndMerge(ActsFatrasSimToolCfg(flags)),
        acc.popToolsAndMerge(ParticleKillerToolCfg(flags)),
        acc.popToolsAndMerge(FastCaloSimV2ToolCfg(flags)),
        acc.popToolsAndMerge(ATLFAST_Geant4ToolCfg(flags))
    ])

    acc.merge(Kernel_GenericSimulatorMTCfg(flags, name, **kwargs))
    return acc


def Kernel_CosmicsG4Cfg(flags, name="ISF_Kernel_CosmicsG4", **kwargs):
    acc = ComponentAccumulator()
    from ISF_Tools.ISF_ToolsConfig import CosmicEventFilterToolCfg
    kwargs.setdefault("EventFilterTools"            , [ acc.addPublicTool(acc.popToolsAndMerge(CosmicEventFilterToolCfg(flags))) ]   )
    kwargs.setdefault("ParticleBroker", acc.getPrimaryAndMerge(ParticleBrokerSvcNoOrderingCfg(flags)).name)
    kwargs.setdefault("MaximumParticleVectorSize"   , 1000000)
    pubTool = acc.addPublicTool(acc.popToolsAndMerge(FullGeant4SelectorCfg(flags)))
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ pubTool ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ pubTool ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ pubTool ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ pubTool ] )
    kwargs.setdefault("CavernSimulationSelectors"   , [ pubTool ] )
    kwargs.setdefault("DoMemoryMonitoring", False)
    acc.merge(Kernel_GenericSimulatorCfg(flags, name, **kwargs)) # Merge properly configured SimKernel here and let deduplication sort it out.
    return acc


def ISF_KernelCfg(flags):
    cfg = ComponentAccumulator()
    # Write MetaData container
    cfg.merge(writeSimulationParametersMetadata(flags))
    # Also allow reading it
    cfg.merge(readSimulationParameters(flags))  # for FileMetaData creation

    from SimulationConfig.SimEnums import SimulationFlavour
    if flags.Sim.ISF.Simulator is SimulationFlavour.FullG4MT:
        cfg.merge(Kernel_FullG4MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.FullG4MT_QS:
        cfg.merge(Kernel_FullG4MT_QSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.PassBackG4MT:
        cfg.merge(Kernel_PassBackG4MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3MT:
        cfg.merge(Kernel_ATLFAST3MTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3MT_QS:
        cfg.merge(Kernel_ATLFAST3MT_QSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3F_G4MS:
        cfg.merge(Kernel_ATLFAST3F_G4MSCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.ATLFAST3F_ACTSMT:
        cfg.merge(Kernel_ATLFAST3F_ACTSMTCfg(flags))
    elif flags.Sim.ISF.Simulator is SimulationFlavour.CosmicsG4:
        cfg.merge(Kernel_CosmicsG4Cfg(flags))
    else:
        raise ValueError('Unknown Simulator set, bailing out')

    return cfg
