# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from SimulationConfig.SimEnums import  BeamPipeSimMode, CalibrationRun, LArParameterization


def enableFrozenShowersFCalOnly(flags):
    """Turns on GFlash shower parametrization for FCAL"""
    flags.Sim.LArParameterization = LArParameterization.FrozenShowersFCalOnly
    flags.Sim.CalibrationRun = CalibrationRun.Off


def disableFrozenShowersFCalOnly(flags):
    """Turns off GFlash shower parametrization for FCAL"""
    flags.Sim.LArParameterization = LArParameterization.NoFrozenShowers


def enableBeamPipeKill(flags):
    flags.Sim.BeamPipeCut = 0.
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim


def disableBeamPipeKill(flags):
    flags.Sim.BeamPipeCut = 100.0
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.Normal


def enableTightMuonStepping(flags):
    flags.Sim.TightMuonStepping = True


def disableTightMuonStepping(flags):
    flags.Sim.TightMuonStepping = False


def enableMuonFieldOnlyInCalo(flags):
    """Activate magnetic field switch off in central LAr calorimeter"""
    flags.Sim.MuonFieldOnlyInCalo = True


def disableMuonFieldOnlyInCalo(flags):
    flags.Sim.MuonFieldOnlyInCalo = False


def enablePhotonRussianRoulette(flags):
    """Activate Photon Russian Roulette: Fast simulation" killing low energy photons with some probability."""
    flags.Sim.PRRThreshold = 0.5  # MeV
    flags.Sim.PRRWeight = 10.


def disablePhotonRussianRoulette(flags):
    flags.Sim.PRRThreshold = False
    flags.Sim.PRRWeight = False


def enableNeutronRussianRoulette(flags):
    """Activate Neutron Russian Roulette: Fast simulation" killing low energy neutrons with some probability."""
    flags.Sim.NRRThreshold = 2.  # MeV
    flags.Sim.NRRWeight = 10.
    flags.Sim.CalibrationRun = CalibrationRun.Off


def disableNeutronRussianRoulette(flags):
    flags.Sim.NRRThreshold = False
    flags.Sim.NRRWeight = False


def enableEMRangeCuts(flags):
    """Turn on range cuts for gamma processes (conv, phot, compt)"""
    flags.Sim.G4Commands += ['/process/em/applyCuts true']


def disableEMRangeCuts(flags):
    commands = flags.Sim.G4Commands
    commands.remove("/process/em/applyCuts true")
    flags.Sim.G4Commands = commands


def enableG4GammaGeneralProcess(flags):
    """Activate the G4GammaGeneralProcess and the UserAction required"""
    flags.Sim.G4Commands+=["/process/em/UseGeneralProcess true"]
    flags.Sim.OptionalUserActionList += ['G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg']


def disableG4GammaGeneralProcess(flags):
    """Disables G4GammaGeneralProcess AND Woodcock tracking"""
    commands = flags.Sim.G4Commands
    commands.remove("/process/em/UseGeneralProcess true")
    commands.remove("/process/em/useWoodcockTracking EMEC") # Woodcock tracking requires G4GammaGeneralProcess
    flags.Sim.G4Commands = commands
    optionalUserActions = flags.Sim.OptionalUserActionList
    optionalUserActions.remove("G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg")
    flags.Sim.OptionalUserActionList = optionalUserActions


def enableWoodcockTracking(flags):
    """Activate the Woodcock Tracking in the EMEC"""
    flags.Sim.G4Commands += ['/process/em/useWoodcockTracking EMEC']
    # Please note that the Woodcock tracking enables the
    # G4GammaGeneralProcess therefore the FixG4CreatorProcessTool must
    # be added if it's not done before
    if "G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg" not in flags.Sim.OptionalUserActionList:
        flags.Sim.OptionalUserActionList += ['G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg']


def disableWoodcockTracking(flags):
    commands = flags.Sim.G4Commands
    commands.remove("/process/em/useWoodcockTracking EMEC")
    flags.Sim.G4Commands = commands
    if "/process/em/UseGeneralProcess true" not in flags.Sim.G4Commands:
        optionalUserActions = flags.Sim.OptionalUserActionList
        optionalUserActions.remove("G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg")
        flags.Sim.OptionalUserActionList = optionalUserActions


def enableG4Optimizations(flags):
    """Enable G4 optimizations"""

    # Activate magnetic field switch off in central LAr calorimeter
    #  More info: https://its.cern.ch/jira/browse/ATLPHYSVAL-773
    enableMuonFieldOnlyInCalo(flags)

    # Photon Russian Roulette
    # "Fast simulation" killing low energy photons with some probability.
    #  More info: https://its.cern.ch/jira/browse/ATLASSIM-4096
    enablePhotonRussianRoulette(flags)

    # Neutron Russian Roulette
    # "Fast simulation" killing low energy neutrons with some probability.
    #  More info: its.cern.ch/jira/browse/ATLASSIM-3924
    enableNeutronRussianRoulette(flags)

    # EM Range Cuts
    # Turn on range cuts for gamma processes (conv, phot, compt)
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-3956
    enableEMRangeCuts(flags)

    # G4GammaGeneralProcess
    # Activate the G4GammaGeneralProcess and the UserAction required
    # to fix the creator process of secondary tracks.
    enableG4GammaGeneralProcess(flags)

    # Energy Loss fluctuation OFF
    # Switch off the Energy loss fluctuation process
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-6995
    # Main physics validation: https://its.cern.ch/jira/browse/ATLPHYSVAL-1009
    # Follow-up validation for Muons:  https://its.cern.ch/jira/browse/ATLPHYSVAL-1022
    # Follow-up validation for Egamma: https://its.cern.ch/jira/browse/ATLPHYSVAL-1030
    # This optimization is on hold for mc23e - for discrepancies seen in the
    # follow-up validations by EgammaCP group.
    # It will be evaluated for future campaigns.
    # flags.Sim.G4Commands+=["/process/eLoss/fluct false"]

    # Activate the Woodcock Tracking in the EMEC
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-5079
    enableWoodcockTracking(flags)


def WoodcockTrackingInEMEC(flags):
    # Use Woodcock Tracking in the EMEC rather than the EMECPara
    # G4Region. This preInclude should be added at the end of the list
    # of preIncludes.
    commands = flags.Sim.G4Commands
    commands.remove("/process/em/useWoodcockTracking EMECPara")
    flags.Sim.G4Commands = commands + ["/process/em/useWoodcockTracking EMEC"]


def WoodcockTrackingInEMECPara(flags):
    # Use Woodcock Tracking in the EMECPara rather than the EMEC
    # G4Region. This preInclude should be added at the end of the list
    # of preIncludes.
    commands = flags.Sim.G4Commands
    commands.remove("/process/em/useWoodcockTracking EMEC")
    flags.Sim.G4Commands = commands + ["/process/em/useWoodcockTracking EMECPara"]


def PostIncludeTweakPhysicsRegionsCfg(flags, cfg):
    # This postInclude drops BeamPipe::SectionF198 and
    # BeamPipe::SectionF199 from the DeadMaterial G4Region, to avoid a
    # clash with the BeampipeFwdCut G4Region.
    from AthenaConfiguration.ComponentAccumulator import ConfigurationError
    detConTool = None
    try:
        detConTool = cfg.getPublicTool('G4AtlasDetectorConstructionTool')
    except ConfigurationError:
        pass
    if detConTool is None:
        return
    detConTool.RegionCreators['DeadMaterialPhysicsRegionTool'].VolumeList.remove('BeamPipe::SectionF198')
    detConTool.RegionCreators['DeadMaterialPhysicsRegionTool'].VolumeList.remove('BeamPipe::SectionF199')
