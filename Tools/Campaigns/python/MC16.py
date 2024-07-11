# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign


def MC16a(flags):
    """MC16a flags for MC to match 2015 and 2016 data"""
    flags.Input.MCCampaign = Campaign.MC16a

    flags.Beam.NumberOfCollisions = 20.

    from LArConfiguration.LArConfigRun2 import LArConfigRun2PileUp
    LArConfigRun2PileUp(flags)
    flags.Digitization.HighGainEMECIW = True

    # pile-up
    flags.Digitization.PU.NumberOfLowPtMinBias = 44.3839246425
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.116075313
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2015'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run284500_MC16a'


def MC16d(flags):
    """MC16d flags for MC to match 2017 data"""
    flags.Input.MCCampaign = Campaign.MC16d

    flags.Beam.NumberOfCollisions = 20.

    from LArConfiguration.LArConfigRun2 import LArConfigRun2PileUp
    LArConfigRun2PileUp(flags)

    # pile-up
    flags.Digitization.PU.NumberOfLowPtMinBias = 80.290021063135
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.2099789464
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run300000_MC16d'


def MC16e(flags):
    """MC16e flags for MC to match 2018 data"""
    flags.Input.MCCampaign = Campaign.MC16e

    flags.Beam.NumberOfCollisions = 20.

    from LArConfiguration.LArConfigRun2 import LArConfigRun2PileUp
    LArConfigRun2PileUp(flags)

    # pile-up
    flags.Digitization.PU.NumberOfLowPtMinBias = 99.2404608
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.2595392
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run310000_MC16e'


def MC16NoPileUp(flags):
    """MC16 flags for MC without pile-up"""
    flags.Input.MCCampaign = Campaign.MC16a

    flags.Beam.NumberOfCollisions = 0.

    from LArConfiguration.LArConfigRun2 import LArConfigRun2NoPileUp
    LArConfigRun2NoPileUp(flags)


def MC16SimulationNoIoV(flags):
    """MC16 flags for simulation without specifying conditions IoVs"""
    flags.Input.MCCampaign = Campaign.MC16a

    from SimulationConfig.SimEnums import TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TRTRangeCut = 30.0
    flags.Sim.TightMuonStepping = True

    from SimulationConfig.G4Optimizations import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    if flags.Sim.ISF.Simulator.isFullSim():
        enableFrozenShowersFCalOnly(flags)


def MC16SimulationSingleIoV(flags):
    """MC16 flags for Simulation"""
    MC16SimulationNoIoV(flags)

    flags.Input.RunNumbers = [284500]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC16Simulation(flags):
    """MC16 flags for Simulation (alias)"""
    MC16SimulationSingleIoV(flags)
