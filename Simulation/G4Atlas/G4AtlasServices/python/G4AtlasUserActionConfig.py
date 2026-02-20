# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import MeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from CaloG4Sim.CaloG4SimConfig import CalibrationDefaultProcessingToolCfg
from SimulationConfig.SimEnums import CalibrationRun, CavernBackground, SimulationFlavour
from G4CosmicFilter.G4CosmicFilterConfig import CosmicFilterToolCfg, StoppedParticleFilterToolCfg
from G4UserActions.G4UserActionsConfig import (
    AthenaStackingActionToolCfg, AthenaTrackingActionToolCfg,
    G4SimTimerToolCfg, G4TrackCounterToolCfg, HitWrapperToolCfg,
    LooperKillerToolCfg, StoppedParticleActionToolCfg
)
from G4RunManagement.G4RunManagementUAConfig import SyncPrimaryGeneratorActionToolCfg, SyncRunActionToolCfg, SyncEventActionToolCfg
from ISF_Geant4CommonTools.ISF_Geant4CommonToolsConfig import EntryLayerToolCfg, EntryLayerToolMTCfg
from ISF_Services.ISF_ServicesCoreConfig import ATLFAST_GeoIDSvcCfg, GeoIDSvcCfg
from ISF_Services.ISF_ServicesConfig import (
    ATLFAST_ParticleBrokerSvcCfg, ParticleBrokerSvcCfg, TruthServiceCfg
)
from MCTruthBase.MCTruthBaseConfig import MCTruthSteppingActionToolCfg


def OptionalUserActionCfg(flags):
    """ flags.Sim.OptionalUserActionList = ['G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg']"""
    result = ComponentAccumulator()
    optionalUserActions = []
    for userActionString in flags.Sim.OptionalUserActionList:
        optionalUserActions += [result.popToolsAndMerge(getOptionalUACfg(flags, userActionString))]
    result.setPrivateTools(optionalUserActions)
    return result


def getOptionalUACfg(flags, userActionString):
    """Execute a function to configure and optional UserAction"""
    parts = userActionString.split('.')
    if len(parts) < 2:
        raise ValueError('OptionalUserAction strings should be of the form Package.Module.Function or Package.Function if defined in __init__.py')
    function = parts[-1]
    module = '.'.join(parts[:-1])
    from importlib import import_module
    loaded_module = import_module(module)
    function_def = getattr(loaded_module, function)
    return function_def(flags)


# Pulled in from ISF G4 to avoid circular dependence
def FullG4TrackProcessorUserActionToolCfg(flags, name="FullG4TrackProcessorUserActionTool", **kwargs):
    result = ComponentAccumulator()
    if flags.Sim.ISF.Simulator in [SimulationFlavour.FullG4MT, SimulationFlavour.FullG4MT_QS]:
        tool = result.popToolsAndMerge(EntryLayerToolMTCfg(flags))
    else:
        tool = result.popToolsAndMerge(EntryLayerToolCfg(flags))
    kwargs.setdefault("EntryLayerTool", result.addPublicTool(tool))
    kwargs.setdefault("GeoIDSvc", result.getPrimaryAndMerge(GeoIDSvcCfg(flags)))
    if flags.Detector.GeometryCavern:
        kwargs.setdefault("TruthVolumeLevel", 2)
    kwargs.setdefault("IsITkGeometry", flags.GeoModel.Run >= LHCPeriod.Run4)
    result.setPrivateTools(CompFactory.G4UA.iGeant4.TrackProcessorUserActionFullG4Tool(name, **kwargs))
    return result


def PhysicsValidationUserActionToolCfg(flags, name="ISFG4PhysicsValidationUserActionTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.iGeant4.PhysicsValidationUserActionTool(name, **kwargs))
    return result


def MCTruthUserActionToolCfg(flags, name="ISFMCTruthUserActionTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("TruthRecordSvc", result.getPrimaryAndMerge(TruthServiceCfg(flags)))
    result.setPrivateTools(CompFactory.G4UA.iGeant4.MCTruthUserActionTool(name, **kwargs))
    return result


def TrackProcessorUserActionToolCfg(flags, name="ISFG4TrackProcessorUserActionTool", **kwargs):
    result = ComponentAccumulator()
    if "ParticleBroker" not in kwargs:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ParticleBrokerSvcCfg(flags)))
    kwargs.setdefault("GeoIDSvc", result.getPrimaryAndMerge(GeoIDSvcCfg(flags)))
    result.setPrivateTools(CompFactory.G4UA.iGeant4.TrackProcessorUserActionPassBackTool(name, **kwargs))
    return result


def PassBackG4TrackProcessorUserActionToolCfg(flags, name="PassBackG4TrackProcessorUserActionTool", **kwargs):
    if flags.Sim.ISF.Simulator in [SimulationFlavour.PassBackG4MT]:
        kwargs.setdefault("ParticleBroker", "")
    return TrackProcessorUserActionToolCfg(flags, name, **kwargs)


def ATLFAST_G4TrackProcessorUserActionToolCfg(flags, name="ATLFAST_G4TrackProcessorUserActionTool", **kwargs):
    result = ComponentAccumulator()
    if flags.Sim.ISF.Simulator in [SimulationFlavour.PassBackG4MT, SimulationFlavour.ATLFASTIIMT, SimulationFlavour.ATLFAST3MT, SimulationFlavour.ATLFAST3MT_QS]:
        kwargs.setdefault("ParticleBroker", "")
    elif flags.Sim.ISF.Simulator in [SimulationFlavour.ATLFASTIIF_G4MS, SimulationFlavour.ATLFAST3F_G4MS]:
        kwargs.setdefault("ParticleBroker", result.getPrimaryAndMerge(ATLFAST_ParticleBrokerSvcCfg(flags)))
    kwargs.setdefault("GeoIDSvc", result.getPrimaryAndMerge(ATLFAST_GeoIDSvcCfg(flags)))
    kwargs.setdefault("PassBackEkinThreshold", 0.05*MeV)
    kwargs.setdefault("KillBoundaryParticlesBelowThreshold", True)
    tool = result.popToolsAndMerge(PassBackG4TrackProcessorUserActionToolCfg(flags, name, **kwargs))
    result.setPrivateTools(tool)
    return result


# New function for all user action types
def getDefaultActions(flags):
    result = ComponentAccumulator()

    actions = []

    # Sync actions should be added first
    # TODO: have dedicated array/handle for sync actions instead of relying on order
    if flags.Sim.UseG4Workers:
         actions += [result.popToolsAndMerge(SyncPrimaryGeneratorActionToolCfg(flags)),
                     result.popToolsAndMerge(SyncRunActionToolCfg(flags)),
                     result.popToolsAndMerge(SyncEventActionToolCfg(flags))]

    # System stacking action
    actions += [result.popToolsAndMerge(AthenaStackingActionToolCfg(flags))]

    # Some truth handling actions (and timing)
    if not flags.Sim.ISFRun:
        actions += [result.popToolsAndMerge(AthenaTrackingActionToolCfg(flags)),
                    result.popToolsAndMerge(MCTruthSteppingActionToolCfg(flags)),
                    result.popToolsAndMerge(G4SimTimerToolCfg(flags))]
    # Track counter
    actions += [result.popToolsAndMerge(G4TrackCounterToolCfg(flags))]

    # Cosmic Perigee action
    if flags.Beam.Type is BeamType.Cosmics and flags.Sim.CavernBackground is CavernBackground.Off:
        actions += [CompFactory.G4UA.CosmicPerigeeActionTool()]
    # Cosmic filter
    if flags.Beam.Type is BeamType.Cosmics and not flags.Sim.ISFRun:
        actions += [result.popToolsAndMerge(CosmicFilterToolCfg(flags))]
    if flags.Sim.StoppedParticleFile:
        actions += [result.popToolsAndMerge(StoppedParticleFilterToolCfg(flags)),
                    result.popToolsAndMerge(StoppedParticleActionToolCfg(flags))]
    # Hit wrapper action
    if flags.Sim.CavernBackground is CavernBackground.Read:
        actions += [result.popToolsAndMerge(HitWrapperToolCfg(flags))]
    # Photon killer
    if flags.Sim.PhysicsList == "QGSP_BERT_HP":
        actions += [CompFactory.G4UA.PhotonKillerTool()]
    # Calo calibration default processing
    if flags.Sim.CalibrationRun in [CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        actions += [result.popToolsAndMerge(CalibrationDefaultProcessingToolCfg(flags))]
    if 'G4UserActions.G4UserActionsConfig.MonopoleLooperKillerToolCfg' not in flags.Sim.OptionalUserActionList:
        actions += [result.popToolsAndMerge(LooperKillerToolCfg(flags))]

    result.setPrivateTools(actions)
    return result


def UserActionSvcCfg(flags, name="G4UA::UserActionSvc", **kwargs):
    """
    Get the standard UA svc configurable with all default actions added.
    This function is normally called by the configured factory, not users.
    """
    result = ComponentAccumulator()

    generalActions = ( result.popToolsAndMerge(getDefaultActions(flags)) +
                       result.popToolsAndMerge(OptionalUserActionCfg(flags)) )

    # New user action tools
    kwargs.setdefault("UserActionTools", generalActions)

    # placeholder for more advanced config, if needed
    result.addService(CompFactory.G4UA.UserActionSvc(name, **kwargs), primary = True)

    return result


def CTBUserActionSvcCfg(flags, name="G4UA::CTBUserActionSvc", **kwargs):
    result = ComponentAccumulator()
    # FIXME migrate an alternative to this
    generalActions = ( result.popToolsAndMerge(getDefaultActions(flags)) +
                       result.popToolsAndMerge(OptionalUserActionCfg(flags)) )
    # This comment carried over from old style:
    # FIXME: ADS these actions are not yet migrated to Hive
    #if simFlags.SimLayout.get_Value()=="tb_LArH6_2004":
    #    eoe=["LArHitsH6EventAction"]+eoe
    #    eoe+=["LArGeoH62004EventAction"]
    #    if simFlags.LArTB_H6Step.statusOn:
    #        if simFlags.LArTB_H6Step.get_Value():
    #            stepping+=["LArGeoH62004SteppingAction"]
    #            event+=["RadLenNtuple"]
    #            eoe+=["RadLenNtuple"]
    #            stepping+=["RadLenNtuple"]

    # New user action tools
    kwargs.setdefault("UserActionTools", generalActions)

    # placeholder for more advanced config, if needed
    result.addService(CompFactory.G4UA.UserActionSvc(name, **kwargs), primary = True)
    return result


def ISFUserActionSvcCfg(flags, name="G4UA::ISFUserActionSvc", **kwargs):
    result = ComponentAccumulator()
    TrackProcessorUserAction = kwargs.pop("TrackProcessorUserAction",[])

    PhysicsValidationUserAction = []
    if flags.Sim.ISF.ValidationMode:
        PhysicsValidationUserAction = [result.popPrivateTools(PhysicsValidationUserActionToolCfg(flags))]

    MCTruthUserAction = kwargs.pop("MCTruthUserAction",
                                   [result.popToolsAndMerge(MCTruthUserActionToolCfg(flags))])

    generalActions = ( TrackProcessorUserAction + MCTruthUserAction +
                       result.popToolsAndMerge(getDefaultActions(flags)) +
                       result.popToolsAndMerge(OptionalUserActionCfg(flags)) +
                       PhysicsValidationUserAction )

    # New user action tools
    kwargs.setdefault("UserActionTools", generalActions)
    result.addService(CompFactory.G4UA.UserActionSvc(name, **kwargs), primary = True)
    return result


def ISFFullUserActionSvcCfg(flags, name="G4UA::ISFFullUserActionSvc", **kwargs):
    # this configuration needs ISFMCTruthUserAction
    # and FullG4TrackProcessorUserAction
    tpAcc = FullG4TrackProcessorUserActionToolCfg(flags)
    kwargs.setdefault("TrackProcessorUserAction", [tpAcc.popPrivateTools()])
    result = ISFUserActionSvcCfg(flags, name, **kwargs)
    result.merge(tpAcc)
    return result


def ISFPassBackUserActionSvcCfg(flags, name="G4UA::ISFPassBackUserActionSvc", **kwargs):
    # this configuration needs ISFMCTruthUserAction and
    # PassBackG4TrackProcessorUserAction
    tpAcc = ComponentAccumulator()
    kwargs.setdefault("TrackProcessorUserAction",
                      [tpAcc.popToolsAndMerge(PassBackG4TrackProcessorUserActionToolCfg(flags))])
    result = ISFUserActionSvcCfg(flags, name, **kwargs)
    result.merge(tpAcc)
    return result


def ISF_ATLFAST_UserActionSvcCfg(flags, name="G4UA::ISF_ATLFAST_UserActionSvc", **kwargs):
    # this configuration needs ISFMCTruthUserAction and
    # ATLFAST_G4TrackProcessorUserAction
    tpAcc = ComponentAccumulator()
    kwargs.setdefault("TrackProcessorUserAction",
                      [tpAcc.popToolsAndMerge(ATLFAST_G4TrackProcessorUserActionToolCfg(flags))])
    result = ISFUserActionSvcCfg(flags, name, **kwargs)
    result.merge(tpAcc)
    return result
