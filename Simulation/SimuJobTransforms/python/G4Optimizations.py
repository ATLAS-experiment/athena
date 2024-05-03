# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from SimulationConfig.SimEnums import CalibrationRun


def enableG4Optimizations(flags):
    """Enable G4 optimizations"""
    # Activate magnetic field switch off in central LAr calorimeter
    #  More info: https://its.cern.ch/jira/browse/ATLPHYSVAL-773
    flags.Sim.MuonFieldOnlyInCalo = True

    # Photon Russian Roulette
    # "Fast simulation" killing low energy photons with some probability.
    #  More info: https://its.cern.ch/jira/browse/ATLASSIM-4096
    flags.Sim.PRRThreshold = 0.5  # MeV
    flags.Sim.PRRWeight = 10.

    # Neutron Russian Roulette
    # "Fast simulation" killing low energy neutrons with some probability. 
    #  More info: its.cern.ch/jira/browse/ATLASSIM-3924
    flags.Sim.NRRThreshold = 2.  # MeV
    flags.Sim.NRRWeight = 10.
    flags.Sim.CalibrationRun = CalibrationRun.Off

    # EM Range Cuts
    # Turn on range cuts for gamma processes (conv, phot, compt)
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-3956
    flags.Sim.G4Commands += ['/process/em/applyCuts true']

    # G4GammaGeneralProcess
    # Activate the G4GammaGeneralProcess and the UserAction required
    # to fix the creator process of secondary tracks.
    flags.Sim.G4Commands+=["/process/em/UseGeneralProcess true"]
    flags.Sim.OptionalUserActionList += ['G4UserActions.G4UserActionsConfig.FixG4CreatorProcessToolCfg']

    # Energy Loss fluctuation OFF
    # Switch off the Energy loss fluctuation process
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-6995
    flags.Sim.G4Commands+=["/process/eLoss/fluct false"]


    # Activate the Woodcock Tracking in the EMEC
    # Please note that the Woodcock tracking enables
    # the G4GammaGeneralProcess therefore the FixG4CreatorProcessTool
    # must be added if it's not done before (see lines above) 
    # More info: https://its.cern.ch/jira/browse/ATLASSIM-5079
    flags.Sim.G4Commands+=["/process/em/useWoodcockTracking EMEC"]


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
