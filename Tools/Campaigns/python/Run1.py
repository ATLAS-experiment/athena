# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign


def Run1_2010NoPileUp(flags):
    """flags for MC to match 2010 data without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23a # FIXME

    flags.Beam.NumberOfCollisions = 0.

    flags.Input.RunNumbers = [155697]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value

    from LArConfiguration.LArConfigRun1 import LArConfigRun1NoPileUp
    LArConfigRun1NoPileUp(flags)


def Run1_2011NoPileUp(flags):
    """flags for MC to match 2011 data without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23a # FIXME

    flags.Beam.NumberOfCollisions = 0.

    flags.Input.RunNumbers = [180164]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value

    from LArConfiguration.LArConfigRun1 import LArConfigRun1NoPileUp
    LArConfigRun1NoPileUp(flags)


def Run1_2012NoPileUp(flags):
    """MC16 flags for MC to match 2012 data without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23a # FIXME

    flags.Beam.NumberOfCollisions = 0.

    flags.Input.RunNumbers = [212272]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value

    from LArConfiguration.LArConfigRun1 import LArConfigRun1NoPileUp
    LArConfigRun1NoPileUp(flags)


def Run1_SimulationNoIoV(flags):
    """flags for simulation without specifying conditions IoVs"""
    flags.Input.MCCampaign = Campaign.MC23a

    from SimulationConfig.SimEnums import TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TRTRangeCut = 30.0
    flags.Sim.TightMuonStepping = True

    from SimuJobTransforms.SimulationHelpers import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    if flags.Sim.ISF.Simulator.isFullSim():
        enableFrozenShowersFCalOnly(flags)


def Run1_2010_SimulationSingleIoV(flags):
    """flags for Simulation"""
    Run1_SimulationNoIoV(flags)

    flags.Input.RunNumbers = [155697]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def Run1_2011_SimulationSingleIoV(flags):
    """flags for Simulation"""
    Run1_SimulationNoIoV(flags)

    flags.Input.RunNumbers = [180164]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def Run1_2012_SimulationSingleIoV(flags):
    """flags for Simulation"""
    Run1_SimulationNoIoV(flags)

    flags.Input.RunNumbers = [212272]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value
