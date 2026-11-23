# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ------------------------------------------------------------
# Definition of trigger EDM for Run 4

# Concept of categories is kept similar to TriggerEDMRun3.py, categories are:
# AllowedCategories = ['Bjet', 'Bphys', 'Egamma', 'ID', 'Jet', 'L1', 'MET', 'MinBias', 'Muon', 'Steer', 'Tau', 'Calo', 'UTT']

# ------------------------------------------------------------

from AthenaCommon.Logging import logging
__log = logging.getLogger('TriggerEDMRun4Config')

# ------------------------------------------------------------
# Additional properties for EDM collections
# ------------------------------------------------------------
# from TrigEDMConfig.TriggerEDMDefs import InViews, Alias # Import when needed, allowTruncation
allowedRun4EDMPrefixes = ['EF_', 'L1_']
run4EDMNamingExceptionsList = ['SeedLessFS']

# ------------------------------------------------------------
# Lists of variables to be kept in the collections 
# ------------------------------------------------------------

cPFOVarsToKeep = [
    'IsInDenseEnvironment',
    'TracksExpectedEnergyDeposit',
]
nPFOVarsToKeep = [
    'AVG_LAR_Q', 'AVG_TILE_Q', 'BADLARQ_FRAC',
    'CENTER_LAMBDA', 'CENTER_MAG',
    'EM_PROBABILITY',
    'N_BAD_CELLS', 'ENG_BAD_CELLS', 'ENG_POS',
    'ISOLATION',
    'LAYERENERGY_EMB1', 'LAYERENERGY_EMB2', 'LAYERENERGY_EMB3',
    'LAYERENERGY_EME1', 'LAYERENERGY_EME2', 'LAYERENERGY_EME3',
    'LAYERENERGY_FCAL0', 'LAYERENERGY_FCAL1', 'LAYERENERGY_FCAL2',
    'LAYERENERGY_HEC0', 'LAYERENERGY_HEC1', 'LAYERENERGY_HEC2', 'LAYERENERGY_HEC3',
    'LAYERENERGY_MINIFCAL0', 'LAYERENERGY_MINIFCAL1', 'LAYERENERGY_MINIFCAL2', 'LAYERENERGY_MINIFCAL3',
    'LAYERENERGY_PreSamplerB', 'LAYERENERGY_PreSamplerE',
    'LAYERENERGY_TILE0',
    'LAYERENERGY_TileBar0', 'LAYERENERGY_TileBar1', 'LAYERENERGY_TileBar2',
    'LAYERENERGY_TileExt0', 'LAYERENERGY_TileExt1', 'LAYERENERGY_TileExt2',
    'LAYERENERGY_TileGap1', 'LAYERENERGY_TileGap2', 'LAYERENERGY_TileGap3',
    'SECOND_LAMBDA', 'SECOND_R',
    'TIMING',
]


# ===========
# === Tau ===
def getTauIDVars(name: str, wps: list[str] | None = None, extra_wps: list[str] | None = None) -> list[str]:
    if wps is None: wps = ['VeryLoose', 'Loose', 'Medium', 'Tight']
    if extra_wps is not None: wps += extra_wps
    # Default list of variables required for all triggers, according to the naming convention
    return [f'{name}_{sfx}' for sfx in ['Score', 'ScoreSigTrans'] + wps]

TauJet_vars = []
TauJet_vars += getTauIDVars('GNTau')
TauJet_vars += getTauIDVars('GNTauDev1', wps=['Loose', 'Medium'])
TauJet_vars_str = '.'.join(TauJet_vars)

TauTrack_vars = ['pt', 'eta', 'phi', 'flagSet', 'trackLinks', 'd0TJVA', 'd0SigTJVA', 'z0sinthetaTJVA', 'z0sinthetaSigTJVA']
TauTrack_vars_str = '.'.join(TauTrack_vars)


TriggerHLTListRun4 = [

    # framework/steering
    #('xAOD::TrigDecision#xTrigDecision' ,                    'ESD AODFULL AODSLIM', 'Steer'), 

    # Collections for Run 4 calorimeter studies
    ('xAOD::TrigRingerRingsContainer#L1_Ringer2sigGlobal',  'BS ESD AODFULL', 'Calo'),
    ('xAOD::TrigRingerRingsAuxContainer#L1_Ringer2sigGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('xAOD::TrigEMClusterContainer#L1_CaloClusters2sigGlobal',  'BS ESD AODFULL', 'Calo'),
    ('xAOD::TrigEMClusterAuxContainer#L1_CaloClusters2sigGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('xAOD::TrigRingerRingsContainer#L1_RingerGlobal',  'BS ESD AODFULL', 'Calo'),
    ('xAOD::TrigRingerRingsAuxContainer#L1_RingerGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('xAOD::TrigEMClusterContainer#L1_CaloClustersGlobal',  'BS ESD AODFULL', 'Calo'),
    ('xAOD::TrigEMClusterAuxContainer#L1_CaloClustersGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('CaloCellContainer#SeedLessFS',  'ESD AODFULL', 'Calo'), 
    
    # Placeholder output collections from GlobalSimulation
    ('std::vector<float>#L1_eFEXeta', 'ESD AODFULL', 'L1'),
    ('std::vector<float>#L1_eFEXphi', 'ESD AODFULL', 'L1'),
    ('std::vector<float>#L1_FailedeFEXeta', 'ESD AODFULL', 'L1'),
    ('std::vector<float>#L1_FailedeFEXphi', 'ESD AODFULL', 'L1'),
    ('std::vector<float>#L1_eGamma1BDT', 'ESD AODFULL', 'L1'),

]

#-------------------------------------------------------------------------------
# EDM details list to store the transient-persistent version
#-------------------------------------------------------------------------------
EDMDetailsRun4 = {}

# xAOD versions are auto-detected at serialisation, but T/P classes need to specify P version here
EDMDetailsRun4[ "TrigRoiDescriptorCollection" ]     = {'persistent':"TrigRoiDescriptorCollection_p3"}
EDMDetailsRun4[ "Muon::NSW_TrigRawDataContainer" ]  = {'persistent':"Muon::NSW_TrigRawDataContainer_p1"}

EDMDetailsRun4[ "xAOD::TrigDecisionAuxInfo" ]         = {'parent':"xAOD::TrigDecision"}
EDMDetailsRun4[ "xAOD::CaloClusterTrigAuxContainer" ] = {'parent':"xAOD::CaloClusterContainer"}
