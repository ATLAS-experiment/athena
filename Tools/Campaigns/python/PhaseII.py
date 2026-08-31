# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign

def PhaseIIPileUpBase(flags, collisions=200):
    """Phase-II Upgrade / Run 4 flags for MC with pile-up"""
    flags.Beam.NumberOfCollisions = collisions
    flags.Input.MCCampaign = Campaign.PhaseII

    flags.GeoModel.Align.Dynamic = False  # no dynamic alignment for now

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)
    flags.LAr.ROD.NumberOfCollisions = collisions  # Run 4 default

    flags.Digitization.DoInnerDetectorNoise = False  # disable noise for now


def MC23PhaseIINoPileUp(flags):
    """Phase-II Upgrade / Run 4 flags for MC without pile-up"""
    flags.Beam.NumberOfCollisions = 0.
    flags.Input.MCCampaign = Campaign.PhaseII

    flags.GeoModel.Align.Dynamic = False  # no dynamic alignment for now

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    flags.Digitization.DoInnerDetectorNoise = False  # disable noise for now


def MC23PhaseIIPileUp1(flags):
    """Phase-II Upgrade / Run 4 flags for MC23 and above with pile-up, mu=1, conditions like mu=60"""
    PhaseIIPileUpBase(flags, collisions=60)

    flags.Digitization.PU.NumberOfLowPtMinBias = 1.996
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.004
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.CustomProfile={
        'run': 350060,
        'startmu': 1.0,
        'endmu': 2.0,
        'stepmu': 1.0,
        'startlb': 1,
        'timestamp': 1625110000
    }


def MC23PhaseIIPileUp60(flags):
    """Phase-II Upgrade / Run 4 flags for MC23 and above and above with pile-up, mu=60"""
    PhaseIIPileUpBase(flags, collisions=60)

    ## pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 70.
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 69.863
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.137
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.CustomProfile={
        'run': 350060,
        'startmu': 50.0,
        'endmu': 70.0,
        'stepmu': 1.0,
        'startlb': 1,
        'timestamp': 1625110000
    }


def MC23PhaseIIPileUp140(flags):
    """Phase-II Upgrade / Run 4 flags for MC23 and above with pile-up, mu=140"""
    PhaseIIPileUpBase(flags, collisions=140)

    ## pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 150.
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 149.707
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.293
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.CustomProfile={
        'run': 350140,
        'startmu': 130.0,
        'endmu': 150.0,
        'stepmu': 1.0,
        'startlb': 1,
        'timestamp': 1625120000
    }


def MC23PhaseIIPileUp200(flags):
    """Phase-II Upgrade / Run 4 flags for MC23 and above with pile-up, mu=200"""
    PhaseIIPileUpBase(flags, collisions=200)

    ## pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 210.
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 209.590
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.410
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2017'
    flags.Digitization.PU.CustomProfile={
        'run': 350200,
        'startmu': 190.0,
        'endmu': 210.0,
        'stepmu': 1.0,
        'startlb': 1,
        'timestamp': 1625130000
    }

def MC23PhaseIIPileUpMC21a(flags):
    """Phase-II Upgrade / Run 4 flags for MC23 and above with pile-up, MC21a pile-up profile"""
    PhaseIIPileUpBase(flags, collisions=60)

    flags.Digitization.PU.NumberOfLowPtMinBias = 84.335
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.165
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run242006_MC21a_SingleBeamspot'
    
    
def MC23PhaseIISimulationNoIoV(flags):
    """Phase-II Upgrade / Run 4 flags for simulation"""
    flags.Input.MCCampaign = Campaign.PhaseII
    from SimulationConfig.SimEnums import TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TightMuonStepping = True

    from SimulationConfig.G4Optimizations import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    enableFrozenShowersFCalOnly(flags)
    from SimulationConfig.G4Optimizations import enableG4Optimizations
    enableG4Optimizations(flags)


def MC23PhaseIISimulationSingleIoV(flags):
    """Phase-II Upgrade / Run 4 flags for simulation"""
    MC23PhaseIISimulationNoIoV(flags)

    flags.Input.RunNumbers = [350000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23PhaseIISimulation(flags):
    """Phase-II Upgrade / Run 4 flags for simulation (alias)"""
    MC23PhaseIISimulationSingleIoV(flags)
