# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Tools configurations for ISF
KG Tan, 17/06/2012
"""

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from ISF_Config.ISF_jobProperties import ISF_Flags

############## Input: GenericGenerator ###############
def getInput_GenericGenerator(name="ISF_Input_GenericGenerator", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault('AtRndmGenSvc', simFlags.RandomSvc.get_Value())
    kwargs.setdefault('orders', sorted([]))

    athenaCommonFlags.PoolEvgenInput.set_Off()
    import AthenaCommon.AtlasUnixGeneratorJob # noqa: F401
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    from ParticleGenerator.ParticleGeneratorConf import ParticleGenerator
    algorithm = ParticleGenerator(name+'PG', **kwargs)
    topSequence += algorithm

    # No longer using this, as it's now set by StackFiller.GenEventManipulators
    #from VertexPositionGenerator.VertexPositionGeneratorConf import VertexPositionGenerator
    #topSequence += VertexPositionGenerator(name+'VPG')
    return algorithm

############## Input: GenericFiles ###############
def getInput_GenericFiles(name="ISF_Input_GenericFiles", **kwargs):
    # Takes input file from athenaCommonFlags.PoolEvgenInput
    import AthenaPoolCnvSvc.ReadAthenaPool # noqa: F401
    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.EventPersistencySvc.CnvServices += [ 'AthenaPoolCnvSvc' ]
    ServiceMgr.EventSelector.InputCollections = athenaCommonFlags.PoolEvgenInput.get_Value()
    from AthenaPython import PyAthena
    return PyAthena.Alg(name)

############## Input: MultiParticleTest ###############
def getInput_MultiParticleTest(name="ISF_Input_MultiParticleTest", **kwargs):
    kwargs.setdefault('orders', sorted(['id[0]: sequence -13 13',
                                        'vertX[0]: constant 0.0',
                                        'vertY[0]: constant 0.0',
                                        'vertZ[0]: constant 0.0',
                                        't[0]: constant 0.0',
                                        'eta[0]: flat -2.0 2.0',
                                        'phi[0]: flat  0 6.28318',
                                        'e[0]: constant 50000',
                                        'id[1]: sequence 22',
                                        'vertX[1]: constant 0.0',
                                        'vertY[1]: constant 0.0',
                                        'vertZ[1]: constant 0.0',
                                        't[1]: constant 0.0',
                                        'eta[1]: flat -2.0 2.0',
                                        'phi[1]: flat  0 6.28318',
                                        'e[1]: constant 50000',
                                        'id[2]: sequence -13 13',
                                        'vertX[2]: constant 0.0',
                                        'vertY[2]: constant 0.0',
                                        'vertZ[2]: constant 0.0',
                                        't[2]: constant 0.0',
                                        'eta[2]: flat -2.0 2.0',
                                        'phi[2]: flat  0 6.28318',
                                        'e[2]: constant 50000',
                                        'id[3]: sequence -11 11',
                                        'vertX[3]: constant 0.0',
                                        'vertY[3]: constant 0.0',
                                        'vertZ[3]: constant 0.0',
                                        't[3]: constant 0.0',
                                        'eta[3]: flat -2.0 2.0',
                                        'phi[3]: flat  0 6.28318',
                                        'e[3]: constant 50000',
                                        'id[4]: sequence -211 211',
                                        'vertX[4]: constant 0.0',
                                        'vertY[4]: constant 0.0',
                                        'vertZ[4]: constant 0.0',
                                        't[4]: constant 0.0',
                                        'eta[4]: flat -2.0 2.0',
                                        'phi[4]: flat  0 6.28318',
                                        'e[4]: constant 50000',
                                        ]))
    return getInput_GenericGenerator(name, **kwargs)

############## Input: electrons ###############
def getInput_electrons(name="ISF_Input_electrons", **kwargs):
    kwargs.setdefault('orders', sorted(['pdgcode: sequence 11 -11',
                                        'vertX: constant 0.0',
                                        'vertY: constant 0.0',
                                        'vertZ: constant 0.0',
                                        't: constant 0.0',
                                        'eta: flat -3.0 3.0',
                                        'phi: flat  0 6.28318',
                                        'pt: constant 10000']))
    return getInput_GenericGenerator(name, **kwargs)

############## Input: muons ###############
def getInput_muons(name="ISF_Input_muons", **kwargs):
    kwargs.setdefault('orders', sorted(['pdgcode: sequence 13 -13',
                                        'vertX: constant 0.0',
                                        'vertY: constant 0.0',
                                        'vertZ: constant 0.0',
                                        't: constant 0.0',
                                        'eta: flat -4.0 4.0',
                                        'phi: flat  0 6.28318',
                                        'pt: constant 50000']))
    return getInput_GenericGenerator(name, **kwargs)

############## Input: pions ###############
def getInput_pions(name="ISF_Input_pions", **kwargs):
    kwargs.setdefault('orders', sorted(['pdgcode: sequence 211 -211',
                                        'vertX: constant 0.0',
                                        'vertY: constant 0.0',
                                        'vertZ: constant 0.0',
                                        't: constant 0.0',
                                        'eta: flat -4.0 4.0',
                                        'phi: flat  0 6.28318',
                                        'pt: constant 50000']))
    return getInput_GenericGenerator(name, **kwargs)

############## Input: photons ###############
def getInput_photons(name="ISF_Input_photons", **kwargs):
    kwargs.setdefault('orders', sorted(['pdgcode: fixed 22',
                                        'vertX: constant 0.0',
                                        'vertY: constant 0.0',
                                        'vertZ: constant 0.0',
                                        't: constant 0.0',
                                        'eta: flat -4.0 4.0',
                                        'phi: flat  0 6.28318',
                                        'pt: constant 50000']))
    return getInput_GenericGenerator(name, **kwargs)


############## Input: ttbar ###############
def getInput_ttbar(name="ISF_Input_ttbar", **kwargs):
    athenaCommonFlags.PoolEvgenInput = [ '/afs/cern.ch/atlas/offline/ProdData/16.6.X/16.6.7.Y/ttbar_muplusjets-pythia6-7000.evgen.pool.root' ]
    return getInput_GenericFiles(name, **kwargs)

############## Input: minbias ###############
def getInput_minbias(name="ISF_Input_minbias", **kwargs):
    athenaCommonFlags.PoolEvgenInput = [ '/afs/cern.ch/atlas/offline/ProdData/16.6.X/16.6.7.Y/minbias_Inelastic-pythia8-7000.evgen.pool.root' ]
    # TODO:
    # BeamEffectTransform will not work properly in ISF, since they'll
    # be appliead to each single G4Event (ie each individual particle
    # that is handed over to G4)
    from G4AtlasApps.SimFlags import simFlags
    simFlags.EventFilter.set_On()
    simFlags.EventFilter.get_Value()['BeamEffectTransformation'] = True
    return getInput_GenericFiles(name, **kwargs)

############## Input: geantinos ###############
def getInput_geantinos(name="ISF_Input_geantinos", **kwargs):
    kwargs.setdefault('orders', sorted(['pdgcode: fixed 999',
                                        'vertX: constant 0.0',
                                        'vertY: constant 0.0',
                                        'vertZ: constant 0.0',
                                        't: constant 0.0',
                                        'eta: flat -5.0 5.0',
                                        'phi: flat  0 6.28318',
                                        'pt: constant 10000']))
    return getInput_GenericGenerator(name, **kwargs)


#   ISF Inputs defined above
#
#
#
#
#
#
#
#
#
#
#
#   ISF Simulators defined below

############## Simulator: GenericSimulator ###############
def getKernel_GenericSimulator(name="ISF_Kernel_GenericSimulator", **kwargs):
    kwargs.setdefault("InputHardScatterCollection", "BeamTruthEvent")
    kwargs.setdefault("OutputHardScatterTruthCollection", "TruthEvent")
    if ISF_Flags.Simulator.isQuasiStable():
        kwargs.setdefault('QuasiStablePatcher', 'ZeroLifetimePositioner')
    kwargs.setdefault('InputConverter', 'ISF_InputConverter')
    kwargs.setdefault("ParticleBroker", ISF_Flags.ParticleBroker())
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("UseShadowEvent", simFlags.UseShadowEvent())
    if simFlags.UseShadowEvent() and "TruthPreselectionTool" not in kwargs:
        kwargs.setdefault( "TruthPreselectionTool", "TruthPreselectionTool" )
    kwargs.setdefault("TruthRecordService", simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("MemoryMonitoringTool", "ISF_MemoryMonitor")
    kwargs.setdefault("DoCPUMonitoring", ISF_Flags.DoTimeMonitoring())
    kwargs.setdefault("DoMemoryMonitoring", ISF_Flags.DoMemoryMonitoring())


    from ISF_Algorithms.ISF_AlgorithmsConf import ISF__SimKernel
    SimKernel = ISF__SimKernel(name, **kwargs)
    return SimKernel

############## Simulator: GenericSimulatorMT ###############
def getKernel_GenericSimulatorMT(name="ISF_Kernel_GenericSimulatorMT", **kwargs):
    kwargs.setdefault("InputEvgenCollection", "BeamTruthEvent" )
    kwargs.setdefault("OutputTruthCollection", "TruthEvent" )
    kwargs.setdefault("GeoIDSvc", "ISF_GeoIDSvc" )
    if ISF_Flags.Simulator.isQuasiStable():
        kwargs.setdefault('QuasiStablePatcher', 'ZeroLifetimePositioner')
    kwargs.setdefault('InputConverter', 'ISF_InputConverter')

    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("UseShadowEvent", simFlags.UseShadowEvent())
    if simFlags.UseShadowEvent() and "TruthPreselectionTool" not in kwargs:
        kwargs.setdefault( "TruthPreselectionTool", "TruthPreselectionTool" )

    kwargs.setdefault("TruthRecordService", simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("AlwaysUseGeoIDSvc", False)
    #kwargs.setdefault("MemoryMonitoringTool", "ISF_MemoryMonitor")
    #kwargs.setdefault("DoCPUMonitoring", ISF_Flags.DoTimeMonitoring())
    #kwargs.setdefault("DoMemoryMonitoring", ISF_Flags.DoMemoryMonitoring())

    # Multi-threading settinggs
    from AthenaCommon.ConcurrencyFlags import jobproperties as concurrencyProps
    is_hive = (concurrencyProps.ConcurrencyFlags.NumThreads() > 0)
    if is_hive:
        kwargs.setdefault('Cardinality', concurrencyProps.ConcurrencyFlags.NumThreads())

    from ISF_Algorithms.ISF_AlgorithmsConf import ISF__SimKernelMT
    SimKernelMT = ISF__SimKernelMT(name, **kwargs)
    return SimKernelMT

############## Simulator: GenericSimulatorNoG4 ###############
def getKernel_GenericSimulatorNoG4(name="ISF_Kernel_GenericSimulatorNoG4", **kwargs):
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: GenericSimulatorNoG4MT ###############
def getKernel_GenericSimulatorNoG4MT(name="ISF_Kernel_GenericSimulatorNoG4MT", **kwargs):
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: MultiSimTest ###############
def getKernel_MultiSimTest(name="ISF_Kernel_MultiSimTest", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_DefaultGeant4Selector' ]         )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_ElectronGeant4Selector',
                                                        'ISF_MuonGeant4Selector',
                                                        'ISF_DefaultFatrasSelector' ]         )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_MuonGeant4Selector',
                                                        'ISF_WithinEta5FastCaloSimSelector',
                                                        'ISF_DefaultGeant4Selector' ]         )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultGeant4Selector' ]         )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ] )

    return getKernel_GenericSimulator(name, **kwargs)


############## Simulator: GenericG4Only ###############
def getKernel_GenericG4Only(name="ISF_Kernel_GenericG4Only", **kwargs):
    kwargs.setdefault("MaximumParticleVectorSize"   , 1000000)
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_FullGeant4Selector' ] )
    from G4AtlasApps.SimFlags import simFlags
    if simFlags.SimulateCavern():
        kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_FullGeant4Selector' ] )
    else:
        kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ] )
    if simFlags.CalibrationRun.get_Value() in ['LAr', 'LAr+Tile', 'LAr+Tile+ZDC']:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        kwargs.setdefault("ExtraOutputs", [( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitActive_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )])
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: GenericG4OnlyMT ###############
def getKernel_GenericG4OnlyMT(name="ISF_Kernel_GenericG4OnlyMT", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ] )
    from G4AtlasApps.SimFlags import simFlags
    if simFlags.CalibrationRun.get_Value() in ['LAr', 'LAr+Tile', 'LAr+Tile+ZDC']:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        kwargs.setdefault("ExtraOutputs", [( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitActive_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ), ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )])
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: FullG4 ###############
def getKernel_FullG4(name="ISF_Kernel_FullG4", **kwargs):
    return getKernel_GenericG4Only(name, **kwargs)

############## Simulator: FullG4MT ###############
def getKernel_FullG4MT(name="ISF_Kernel_FullG4MT", **kwargs):
    kwargs.setdefault("SimulationTools", ["ISF_ParticleKillerTool",
                                          "ISF_FullGeant4Tool"])
    return getKernel_GenericG4OnlyMT(name, **kwargs)

############## Simulator: FullG4_QS ###############
def getKernel_FullG4_QS(name="ISF_Kernel_FullG4_QS", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_DefaultLongLivedGeant4Selector' ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_DefaultLongLivedGeant4Selector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_DefaultLongLivedGeant4Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultLongLivedGeant4Selector' ] )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector'  ] )
    return getKernel_GenericG4Only(name, **kwargs)

############## Simulator: FullG4_longLived ###############
def getKernel_FullG4_LongLived(name="ISF_Kernel_FullG4_LongLived", **kwargs): # deprecated name
    return getKernel_FullG4_QS(name, **kwargs)

############## Simulator: FullG4MT_QS ###############
def getKernel_FullG4MT_QS(name="ISF_Kernel_FullG4MT_QS", **kwargs):
    kwargs.setdefault("SimulationTools", ["ISF_ParticleKillerTool",
                                          "ISF_FullGeant4Tool"])
    return getKernel_GenericG4OnlyMT(name, **kwargs)

############## Simulator: FullG4MT_longLived ###############
def getKernel_FullG4MT_LongLived(name="ISF_Kernel_FullG4MT_LongLived", **kwargs): # deprecated name
    return getKernel_FullG4MT_QS(name, **kwargs)

############## Simulator: FullG4_IDOnly ###############
def getKernel_FullG4_IDOnly(name="ISF_Kernel_FullG4_IDOnly", **kwargs):
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_DefaultParticleKillerSelector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultParticleKillerSelector' ] )
    return getKernel_GenericG4Only(name, **kwargs)

############## Simulator: FullG4_IDCalo ###############
def getKernel_FullG4_IDCalo(name="ISF_Kernel_FullG4_IDCalo", **kwargs):
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultParticleKillerSelector' ] )
    return getKernel_GenericG4Only(name, **kwargs)

############## Simulator: PassBackG4 ###############
def getKernel_PassBackG4(name="ISF_Kernel_PassBackG4", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ] )
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: PassBackG4MT ###############
def getKernel_PassBackG4MT(name="ISF_Kernel_PassBackG4MT", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_PassBackGeant4Selector' ] )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ] )
    kwargs.setdefault("SimulationTools"             , [ 'ISF_ParticleKillerTool',
                                                        'ISF_PassBackGeant4Tool' ] )
    kwargs.setdefault("ParticleOrderingTool"        , "ISF_EnergyParticleOrderingTool" )
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: CosmicsG4 ###############
def getKernel_CosmicsG4(name="ISF_Kernel_CosmicsG4", **kwargs):
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_FullGeant4Selector' ] )
    kwargs.setdefault("EventFilterTools"            , [ 'ISF_CosmicEventFilter' ]   )
    return getKernel_FullG4(name, **kwargs)

############## Simulator: FullG4+Stopping Particles ###############
def getKernel_StoppingG4(name="ISF_Kernel_StoppingG4", **kwargs):
    kwargs.setdefault("EventFilterTools"            , [ 'ISF_StoppedParticleFilter' ]   )
    return getKernel_GenericG4Only(name, **kwargs)

############## Simulator: ATLFAST3 #################
def getKernel_ATLFAST3(name="ISF_Kernel_ATLFAST3", **kwargs):
    kwargs.setdefault("ParticleBroker"             , 'ISF_ATLFAST_ParticleBrokerSvc'                    )
    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonATLFAST_Geant4Selector',
                                                       'ISF_NeutronATLFAST_Geant4Selector',
                                                       'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                       'ISF_KLongATLFAST_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ]         )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector' ]        )
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3"
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: ATLFAST3MT ###############
def getKernel_ATLFAST3MT(name="ISF_Kernel_ATLFAST3MT", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonATLFAST_Geant4Selector',
                                                       'ISF_NeutronATLFAST_Geant4Selector',
                                                       'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                       'ISF_KLongATLFAST_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector' ]        )
    kwargs.setdefault("SimulationTools"            , [ 'ISF_ParticleKillerTool',
                                                       'ISF_FastCaloSimV2Tool',
                                                       'ISF_ATLFAST_Geant4Tool'])
    kwargs.setdefault("ParticleOrderingTool"       , 'ISF_ParticleOrderingTool' )
    kwargs.setdefault('EntryLayerTool'             , 'ISF_ATLFAST_EntryLayerToolMT')
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3MT"
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: ATLFAST3MT ###############
def getKernel_ATLFAST3MTEnergyOrdered(name="ISF_Kernel_ATLFAST3MTEnergyOrdered", **kwargs):
    kwargs.setdefault("ParticleOrderingTool"       , 'ISF_EnergyParticleOrderingTool' )
    return getKernel_ATLFAST3MT(name, **kwargs)

############## Simulator: ATLFAST3_QS ###############
def getKernel_ATLFAST3_QS(name="ISF_Kernel_ATLFAST3_QS", **kwargs):
    kwargs.setdefault("ParticleBroker"             , 'ISF_ATLFAST_ParticleBrokerSvc'                        )

    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonATLFAST_Geant4Selector',
                                                       'ISF_NeutronATLFAST_Geant4Selector',
                                                       'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                       'ISF_KLongATLFAST_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector'  ]           )
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3_QS"
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: ATLFAST3MT_QS ###############
def getKernel_ATLFAST3MT_QS(name="ISF_Kernel_ATLFAST3MT_QS", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonATLFAST_Geant4Selector',
                                                       'ISF_NeutronATLFAST_Geant4Selector',
                                                       'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                       'ISF_KLongATLFAST_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector'  ]           )
    kwargs.setdefault("SimulationTools"            , [ 'ISF_ParticleKillerTool',
                                                       'ISF_FastCaloSimV2Tool',
                                                       'ISF_ATLFAST_Geant4Tool'])
    kwargs.setdefault("ParticleOrderingTool"       , 'ISF_ParticleOrderingTool' )
    kwargs.setdefault('EntryLayerTool'             , 'ISF_ATLFAST_EntryLayerToolMT')
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3MT_QS"
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: G4FastCalo ###############
def getKernel_G4FastCalo(name="ISF_Kernel_G4FastCalo", **kwargs):
    kwargs.setdefault("ParticleBroker"             , 'ISF_ATLFAST_ParticleBrokerSvc')
    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonATLFAST_Geant4Selector',
                                                       'ISF_NeutronATLFAST_Geant4Selector',
                                                       'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                       'ISF_KLongATLFAST_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector' ]        )
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "G4FastCalo"
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: ATLFAST3EnergyOrdered ###############
def getKernel_ATLFAST3EnergyOrdered(name="ISF_Kernel_ATLFAST3EnergyOrdered", **kwargs):
    kwargs.setdefault("ParticleBroker"             , 'ISF_ATLFAST_EnergyOrderedParticleBrokerSvc')
    return getKernel_G4FastCalo(name, **kwargs)

############## Simulator: G4FastCaloTest ###############
def getKernel_G4FastCaloTest(name="ISF_Kernel_G4FastCaloTest", **kwargs):
    kwargs.setdefault("ParticleBroker"             , 'ISF_ATLFAST_ParticleBrokerSvc')

    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_PionATLFAST_Geant4Selector',
                                                       'ISF_ProtonG4FastCalo_Geant4Selector',
                                                       'ISF_ChargedKaonG4FastCalo_Geant4Selector',
                                                       'ISF_KLongG4FastCalo_Geant4Selector',
                                                       'ISF_DefaultFastCaloSimV2Selector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector' ]        )
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "G4FastCaloTest"
    return getKernel_G4FastCalo(name, **kwargs)

############## Simulator: G4FastCaloDNN ###############
# like G4FastCalo, replacing FastCaloSimV2 by DNNCaloSim
def getKernel_G4FastCaloDNN(name="ISF_Kernel_G4FastCaloDNN", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors", [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("IDSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CaloSimulationSelectors"    , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                       'ISF_EtaGreater5ParticleKillerSimSelector',
                                                       'ISF_DefaultDNNCaloSimSelector' ] )
    kwargs.setdefault("MSSimulationSelectors"      , [ 'ISF_DefaultATLFAST_Geant4Selector' ]            )
    kwargs.setdefault("CavernSimulationSelectors"  , [ 'ISF_DefaultParticleKillerSelector' ]        )
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "G4FastCaloDNN"
    return getKernel_GenericSimulator(name, **kwargs)


############## Simulator: ATLFAST3F_G4MS ###############
def getKernel_ATLFAST3F_G4MS(name="ISF_Kernel_ATLFAST3F_G4MS", **kwargs):
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_DefaultParticleKillerSelector' ]       )
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_DefaultFatrasSelector' ]               )
    kwargs.setdefault("CaloSimulationSelectors",      [ 'ISF_MuonATLFAST_Geant4Selector',
                                                        'ISF_EtaGreater5ParticleKillerSimSelector',
                                                        'ISF_PionATLFAST_Geant4Selector',
                                                        'ISF_ProtonATLFAST_Geant4Selector',
                                                        'ISF_NeutronATLFAST_Geant4Selector',
                                                        'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                        'ISF_KLongATLFAST_Geant4Selector',
                                                        'ISF_DefaultFastCaloSimV2Selector'])
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultATLFAST_Geant4Selector' ]           )
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ]       )
    # set the simFlags accordingly (TODO: is this even needed?)
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3F_G4MS"
    return getKernel_GenericSimulator(name, **kwargs)

############## Simulator: ATLFAST3F_ACTSMT ###############
def getKernel_ATLFAST3F_ACTSMT(name="ISF_Kernel_ATLFAST3F_ACTSMT", **kwargs):
    kwargs.setdefault("SimulationTools", ["ISF_ParticleKillerTool",
                                          "ISF_FastCaloSimV2Tool",
                                          "ISF_ActsFatrasSimTool",
                                          "ISF_ATLFAST_Geant4Tool"])
    kwargs.setdefault("ParticleOrderingTool"        , 'ISF_ParticleOrderingTool')
    kwargs.setdefault('EntryLayerTool'              , 'ISF_ATLFAST_EntryLayerToolMT')
    kwargs.setdefault("BeamPipeSimulationSelectors" , [ 'ISF_DefaultParticleKillerSelector' ])
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_DefaultActsSelector', 'ISF_DefaultParticleKillerSelector' ])
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_MuonATLFAST_Geant4Selector',
                                                        'ISF_EtaGreater5ParticleKillerSimSelector',
                                                        'ISF_PionATLFAST_Geant4Selector',
                                                        'ISF_ProtonATLFAST_Geant4Selector',
                                                        'ISF_NeutronATLFAST_Geant4Selector',
                                                        'ISF_ChargedKaonATLFAST_Geant4Selector',
                                                        'ISF_KLongATLFAST_Geant4Selector',
                                                        'ISF_DefaultFastCaloSimV2Selector'])
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultATLFAST_Geant4Selector' ])
    kwargs.setdefault("CavernSimulationSelectors"   , [ 'ISF_DefaultParticleKillerSelector' ])
    # set the simFlags accordingly (TODO: is this even needed?)
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFAST3F_ACTSMT"
    return getKernel_GenericSimulatorMT(name, **kwargs)

############## Simulator: DNNOnly ###############
# run DNNCaloSim standalone, for faster tests
def getKernel_DNNOnly(name="ISF_Kernel_DNNOnly", **kwargs):
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_DefaultDNNCaloSimSelector'    ] )
    return getKernel_GenericSimulatorNoG4(name, **kwargs)
