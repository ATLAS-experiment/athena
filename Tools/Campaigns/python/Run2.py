# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign


def Run2_2015_HeavyIons(flags):
    """flags for MC to match 2015 Heavy Ion data"""
    flags.Input.MCCampaign = Campaign.MC23a # FIXME

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 295000
    flags.Input.OverrideRunNumber = True
    flags.Input.RunAndLumiOverrideList = None # Doesn't work??
    from LArConfiguration.LArConfigRun2 import LArConfigRun2NoPileUp
    LArConfigRun2NoPileUp(flags)
    flags.Digitization.HighGainEMECIW = True

    flags.Digitization.PU.NumberOfCavern = 1
    flags.Digitization.PU.NumberOfLowPtMinBias = 0.0
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.0
    #flags.Digitization.PU.CustomProfile={"run":295000, "startmu":0.0, "endmu":0.0, "stepmu":1.0, "startlb":1, "timestamp": 1475000000}
    flags.Digitization.PU.InitialBunchCrossing = 0
    flags.Digitization.PU.FinalBunchCrossing = 0
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_2015_HeavyIons'
    flags.Sim.TRTRangeCut=0.05
