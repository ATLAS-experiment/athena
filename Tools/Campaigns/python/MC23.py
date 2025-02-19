# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import ProductionStep
from Campaigns.Utils import Campaign


def MC23a(flags):
    """MC23a flags for MC to match 2022 Run 3 data"""
    flags.Input.MCCampaign = Campaign.MC23a

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 67.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 67.369
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.131
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC23a_MultiBeamspot'

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1

def MC23c(flags):
    """MC23c flags for MC to match 2023 Run 3 data (initial pile-up profile estimate)"""
    flags.Input.MCCampaign = Campaign.MC23c

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 90.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 90.323
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.177
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run450000_MC23c_MultiBeamspot'

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1


def MC23d(flags):
    """MC23d flags for MC to match 2023 Run 3 data (uses a pile-up profile based on the actual profile from 2023 data)"""
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 95.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 95.313
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.187
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run450000_MC23d_MultiBeamspot' 

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1


def MC23HeavyIons2023NoPileUp(flags):
    """MC23 flags for the 2023 Heavy Ions run (without pile-up)"""
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Beam.BunchSpacing = 50
    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 460000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags) # TO CHECK is it an issue if this is set for RDOtoRDOTrigger?
    flags.Reco.EnableZDC = False # TO CHECK is this actually needed? I think it should be False by default

    #all
    flags.Trigger.AODEDMSet = 'AODFULL'
    flags.Trigger.triggerMenuSetup = 'Dev_HI_run3_v1_TriggerValidation_prescale'
    flags.Trigger.L1.doAlfaCtpin = True


def MC23HeavyIons2023(flags):
    """MC23 flags for the 2023 Heavy Ions run"""
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 460000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags) # TO CHECK is this actually what we want c.f. LArConfigRun3PileUp

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    flags.Digitization.PileUp = True
    flags.Digitization.DoXingByXingPileUp = True
    flags.Digitization.PU.BunchStructureConfig = "RunDependentSimData.BunchStructureHeavyIon2022"
    flags.Digitization.PU.InitialBunchCrossing = 0
    flags.Digitization.PU.FinalBunchCrossing = 0
    flags.Digitization.PU.NumberOfCavern = 1 # We are using the Cavern Background input for the Hijing HITS-level events

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags) # TO CHECK is it an issue if this is set for RDOtoRDOTrigger?
    flags.Reco.EnableZDC = False # TO CHECK is this actually needed? I think it should be False by default

    #all
    flags.Trigger.AODEDMSet = 'AODFULL'
    flags.Trigger.triggerMenuSetup = 'Dev_HI_run3_v1_TriggerValidation_prescale'
    flags.Trigger.L1.doAlfaCtpin = True


def MC23e(flags):
    """MC23e flags for MC to match 2024 Run 3 data (initial pile-up estimate based on broadened 2023 data )"""
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Beam.NumberOfCollisions = 60.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    # These numbers are based upon a relative XS scaling of the high-pt slice
    # of 64%, which leads to a relative high-pt / low-pt sampling of
    # 0.001953314389 / 0.9980466856. Those numbers are then multiplied by 98.5
    # to follow pile-up profile. Only a relevant number of significant digits
    # are kept.
    flags.Digitization.PU.NumberOfLowPtMinBias = 98.308
    flags.Digitization.PU.NumberOfHighPtMinBias = 0.192
    # TODO new bunch structure?
    flags.Digitization.PU.BunchStructureConfig = 'RunDependentSimData.BunchStructure_Fill7314_BCMSPattern_Flat'
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run470000_MC23e_MultiBeamspot' 

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        # ensure better randomisation of high-pt minbias events
        flags.Digitization.PU.HighPtMinBiasInputColOffset = -1


def MC23ppReferenceRun2024(flags): # FIXME This configuration is a placeholder
    """MC23 flags for the 2024 5.36 TeV pp reference run"""
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 488000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags) # TO CHECK is this actually what we want c.f. LArConfigRun3PileUp

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    flags.Digitization.PileUp = True
    flags.Digitization.DoXingByXingPileUp = True
    flags.Digitization.PU.BunchStructureConfig = "RunDependentSimData.BunchStructureHeavyIon2022"
    flags.Digitization.PU.InitialBunchCrossing = 0
    flags.Digitization.PU.FinalBunchCrossing = 0
    flags.Digitization.PU.NumberOfCavern = 1 # We are using the Cavern Background input for the Hijing HITS-level events

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags) # TO CHECK is it an issue if this is set for RDOtoRDOTrigger?
    flags.Reco.EnableZDC = False # TO CHECK is this actually needed? I think it should be False by default

    #all
    flags.Trigger.AODEDMSet = 'AODFULL'
    flags.Trigger.triggerMenuSetup = 'Dev_HI_run3_v1_TriggerValidation_prescale'
    flags.Trigger.L1.doAlfaCtpin = True


def MC23HeavyIons2024NoPileUp(flags): # FIXME This configuration is a placeholder
    """MC23 flags for the 2024 Heavy Ions run (without pile-up)"""
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Beam.BunchSpacing = 50
    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 488600

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags) # TO CHECK is it an issue if this is set for RDOtoRDOTrigger?
    flags.Reco.EnableZDC = False # TO CHECK is this actually needed? I think it should be False by default

    #all
    flags.Trigger.AODEDMSet = 'AODFULL'
    flags.Trigger.triggerMenuSetup = 'Dev_HI_run3_v1_TriggerValidation_prescale'
    flags.Trigger.L1.doAlfaCtpin = True


def MC23HeavyIons2024(flags): # FIXME This configuration is a placeholder
    """MC23 flags for the 2024 Heavy Ions run"""
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 488600

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags) # TO CHECK is this actually what we want c.f. LArConfigRun3PileUp

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential

    # pile-up
    flags.Digitization.PileUp = True
    flags.Digitization.DoXingByXingPileUp = True
    flags.Digitization.PU.BunchStructureConfig = "RunDependentSimData.BunchStructureHeavyIon2022"
    flags.Digitization.PU.InitialBunchCrossing = 0
    flags.Digitization.PU.FinalBunchCrossing = 0
    flags.Digitization.PU.NumberOfCavern = 1 # We are using the Cavern Background input for the Hijing HITS-level events

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags) # TO CHECK is it an issue if this is set for RDOtoRDOTrigger?
    flags.Reco.EnableZDC = False # TO CHECK is this actually needed? I think it should be False by default

    #all
    flags.Trigger.AODEDMSet = 'AODFULL'
    flags.Trigger.triggerMenuSetup = 'Dev_HI_run3_v1_TriggerValidation_prescale'
    flags.Trigger.L1.doAlfaCtpin = True


def MC23aSingleBeamspot(flags):
    """MC23a flags for MC to match 2022 Run 3 data (single beamspot version)"""
    MC23a(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run410000_MC23a_SingleBeamspot'


def MC23cSingleBeamspot(flags):
    """MC23c flags for MC to match 2023 Run 3 data (initial pile-up profile estimate, single beamspot version)"""
    MC23c(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run450000_MC23c_SingleBeamspot'


def MC23dSingleBeamspot(flags):
    """MC23d flags for MC to match 2023 Run 3 data (uses a pile-up profile based on the actual profile from 2023 data, single beamspot version)"""
    MC23d(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run450000_MC23d_SingleBeamspot' 

def MC23eSingleBeamspot(flags):
    """MC23e flags for MC to match 2024 Run 3 data (initial pile-up profile estimate, single beamspot version)"""
    MC23e(flags)

    # override only pile-up profile
    flags.Digitization.PU.ProfileConfig = 'RunDependentSimData.PileUpProfile_run470000_MC23e_SingleBeamspot'

def MC23LowMu(flags):
    """MC23 flags for MC to match Run 3 data with low pile-up"""
    flags.Input.MCCampaign = Campaign.MC23a

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


def MC23NoPileUp(flags):
    """MC23 flags for MC without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23a

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 410000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential


def MC23aNoPileUp(flags):
    """MC23a flags for MC without pile-up"""
    MC23NoPileUp(flags)


def MC23dNoPileUp(flags):
    """MC23d flags for MC without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 450000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential


def MC23eNoPileUp(flags):
    """MC23e flags for MC without pile-up"""
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Beam.NumberOfCollisions = 0.
    flags.Input.ConditionsRunNumber = 470000

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    # radiation damage
    from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
    flags.Digitization.PixelPlanarRadiationDamageSimulationType = PixelRadiationDamageSimulationType.RamoPotential


def MC23NoPileUpLowMuRun(flags):
    """MC23a flags for MC to match 2002 Low Mu data"""
    MC23NoPileUp(flags)
    flags.Input.ConditionsRunNumber = 420000

    
def MC23NoPileUpLowMuLowB(flags):
    """MC23d flags for MC to match special run 460348"""
    MC23NoPileUp(flags)   

    flags.Input.MCCampaign = Campaign.MC23d
    flags.Input.ConditionsRunNumber = 465000

    # B-field configuration
    flags.BField.configuredSolenoidFieldScale = 0.4

    
def BeamspotSplitMC23a():
    """MC23a beamspot splitting configuration"""
    substeps = 4
    event_fractions = [0.14, 0.14, 0.14, 0.58]

    return substeps, event_fractions


def BeamspotSplitMC23c():
    """MC23c beamspot splitting configuration"""
    substeps = 4
    event_fractions = [0.22, 0.22, 0.22, 0.34]

    return substeps, event_fractions


def BeamspotSplitMC23d():
    """MC23d beamspot splitting configuration, matches MC23c, but only the
    first two substep are considered levelling rather than the first
    three."""
    substeps = 4
    event_fractions = [0.22, 0.22, 0.22, 0.34]

    return substeps, event_fractions


def BeamspotSplitMC23e():
    """MC23e beamspot splitting configuration."""
    substeps = 4
    event_fractions = [0.22, 0.22, 0.22, 0.34]

    return substeps, event_fractions


def MC23SimulationNoIoV(flags):
    """MC23 base flags for simulation without specifying conditions IoVs"""
    flags.Input.MCCampaign = Campaign.MC23a

    from SimulationConfig.SimEnums import TruthStrategy
    flags.Sim.PhysicsList = 'FTFP_BERT_ATL'
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus

    flags.Sim.TRTRangeCut = 30.0
    flags.Sim.TightMuonStepping = True

    from SimulationConfig.G4Optimizations import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    if flags.Sim.ISF.Simulator.isFullSim():
        enableFrozenShowersFCalOnly(flags)

    from SimulationConfig.G4Optimizations import enableG4Optimizations
    enableG4Optimizations(flags)

    flags.Sim.FastCalo.ParamsInputFilename = 'FastCaloSim/MC23/TFCSparam_dev_Hybrid_Ha_v5_all_baryons_0_500.root'


def MC23SimulationLowMuRun(flags):
    """MC23 flags for low mu run simulation"""
    MC23SimulationNoIoV(flags)

    flags.Input.RunNumbers = [420000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23Simulation2023HeavyIonRun(flags):
    """MC23 flags for simulation simulation of the 2023 Heavy Ion run"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Input.RunNumber = [460000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23Simulation2024ppRefRun(flags):
    """MC23 flags for simulation simulation of the 2024 5.36 TeV pp reference run"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Input.RunNumber = [488000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23Simulation2024HeavyIonRun(flags):
    """MC23 flags for simulation simulation of the 2024 Heavy Ion run"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Input.RunNumber = [488600]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23dSimulationLowMuLowB(flags):
    """MC23 flags for simulation of special run 460348"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23d

    flags.Input.RunNumber = [465000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumber = [1] # dummy value

    # B-field configuration
    flags.BField.configuredSolenoidFieldScale = 0.4


def MC23SimulationSingleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23a

    flags.Input.RunNumbers = [410000]
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value


def MC23aSimulationMultipleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23a

    flags.Input.OverrideRunNumber = True

    from RunDependentSimComps.PileUpUtils import generateRunAndLumiProfile
    generateRunAndLumiProfile(flags,
                              profile= 'RunDependentSimData.PileUpProfile_run410000_MC23a_MultiBeamspot')


def MC23cSimulationMultipleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23c

    flags.Input.OverrideRunNumber = True

    from RunDependentSimComps.PileUpUtils import generateRunAndLumiProfile
    generateRunAndLumiProfile(flags,
                              profile= 'RunDependentSimData.PileUpProfile_run450000_MC23c_MultiBeamspot')


def MC23eSimulationMultipleIoV(flags):
    """MC23 flags for simulation"""
    MC23SimulationNoIoV(flags)
    flags.Input.MCCampaign = Campaign.MC23e

    flags.Input.OverrideRunNumber = True

    from RunDependentSimComps.PileUpUtils import generateRunAndLumiProfile
    generateRunAndLumiProfile(flags,
                              profile= 'RunDependentSimData.PileUpProfile_run470000_MC23e_MultiBeamspot')


def MC23SimulationSingleIoVCalibrationHits(flags):
    """MC23 flags for simulation with CalibrationHits"""
    MC23SimulationSingleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)


def MC23aSimulationMultipleIoVCalibrationHits(flags):
    """MC23 flags for simulation with CalibrationHits"""
    MC23aSimulationMultipleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)


def MC23cSimulationMultipleIoVCalibrationHits(flags):
    """MC23 flags for simulation with CalibrationHits"""
    MC23cSimulationMultipleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)


def MC23eSimulationMultipleIoVCalibrationHits(flags):
    """MC23e flags for simulation with CalibrationHits"""
    MC23eSimulationMultipleIoV(flags)
    from SimuJobTransforms import CalHits, ParticleID
    CalHits(flags)
    ParticleID(flags)
