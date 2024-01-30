# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import ProductionStep
from Campaigns.Utils import Campaign


def MC21a(flags):
    """MC21a flags for MC to match initial Run 3 data"""
    flags.Input.MCCampaign = Campaign.MC21a

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 84.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 84.335
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.165
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC21a_MultiBeamspot'

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1


def MC21aSingleBeamspot(flags):
    """MC21a flags for MC to match initial Run 3 data (single beamspot version)"""
    MC21a(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC21a_SingleBeamspot'


def MC21LowMu(flags):
    """MC21 flags for MC to match Run 3 data with low pile-up"""
    flags.Input.MCCampaign = Campaign.MC21a

    flags.Beam.NumberOfCollisions = 60.
    flags.Input.ConditionsRunNumber = 410000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 0.05
    # to simulate low pile-up. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 0.0499
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.0001
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'


def MC21NoPileUp(flags):
    """MC21a flags for MC to match initial Run 3 data"""
    flags.Input.MCCampaign = Campaign.MC21a

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 410000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential


def MC21NoPileUpLowMuRun(flags):
    """MC21a flags for MC to match 2002 Low Mu data"""
    MC21NoPileUp(flags)
    flags.Input.ConditionsRunNumber = 420000


def BeamspotSplitMC21a():
    """MC21a beamspot splitting configuration"""
    substeps = 4
    event_fractions = [0.14, 0.14, 0.14, 0.58]

    return substeps, event_fractions


def MC21SimulationNoIoV(flags):
    """MC21 base flags for simulation without specifying conditions IoVs"""
    flags.Input.MCCampaign = Campaign.MC21a

    from SimulationConfig.SimEnums import TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TRTRangeCut = 30.0
    flags.Sim.TightMuonStepping = True

    from SimuJobTransforms.SimulationHelpers import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    if flags.Sim.ISF.Simulator.isFullSim():
        enableFrozenShowersFCalOnly(flags)

    from SimuJobTransforms.G4Optimizations import enableG4Optimizations
    enableG4Optimizations(flags)


def MC21SimulationLowMuRun(flags):
    """MC21 flags for low mu run simulation"""
    MC21SimulationNoIoV(flags)

    flags.Input.RunNumbers = [420000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC21SimulationSingleIoV(flags):
    """MC21 flags for simulation"""
    MC21SimulationNoIoV(flags)

    flags.Input.RunNumbers = [410000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC21Simulation(flags):
    """MC21 flags for simulation (alias)"""
    MC21SimulationSingleIoV(flags)


def MC21SimulationMultipleIoV(flags):
    """MC21 flags for simulation"""
    MC21SimulationNoIoV(flags)

    flags.Input.OverrideRunNumber = True

    from RunDependentSimComps.PileUpUtils import generateRunAndLumiProfile
    generateRunAndLumiProfile(flags,
                              profile= 'RunDependentSimData.PileUpProfile_run410000_MC21a_MultiBeamspot')


def MC21SimulationSingleIoVCalibrationHits(flags):
    """MC21 flags for simulation with CalibrationHits"""
    MC21SimulationSingleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)


def MC21SimulationCalibrationHits(flags):
    """MC21 flags for simulation with CalibrationHits (alias)"""
    MC21SimulationSingleIoVCalibrationHits(flags)


def MC21SimulationMultipleIoVCalibrationHits(flags):
    """MC21 flags for simulation with CalibrationHits"""
    MC21SimulationMultipleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)
