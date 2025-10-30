# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
#from TrigEDMConfig.TriggerEDMDefs import Alias, InViews, allowTruncation # Import when needed

# ----------------------------

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

TriggerHLTListRun4 = [

    # framework/steering
    #('xAOD::TrigDecision#xTrigDecision' ,                    'ESD AODFULL AODSLIM', 'Steer'), 

    # Collections for Run 4 calorimeter studies
    ('xAOD::TrigRingerRingsContainer#Ringer2sigGlobal',  'BS ESD AODFULL', 'Calo'),
    ('xAOD::TrigRingerRingsAuxContainer#Ringer2sigGlobalAux.',  'BS ESD AODFULL', 'Calo'), 

    ('xAOD::TrigEMClusterContainer#CaloClusters2sigGlobal',  'BS ESD AODFULL', 'Calo'), 
    ('xAOD::TrigEMClusterAuxContainer#CaloClusters2sigGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('xAOD::TrigRingerRingsContainer#RingerGlobal',  'BS ESD AODFULL', 'Calo'), 
    ('xAOD::TrigRingerRingsAuxContainer#RingerGlobalAux.',  'BS ESD AODFULL', 'Calo'), 

    ('xAOD::TrigEMClusterContainer#CaloClustersGlobal',  'BS ESD AODFULL', 'Calo'), 
    ('xAOD::TrigEMClusterAuxContainer#CaloClustersGlobalAux.',  'BS ESD AODFULL', 'Calo'),

    ('CaloCellContainer#SeedLessFS',  'ESD AODFULL', 'Calo'), 

    # L1 Calo inputs, note we are giving extended EDM targets
    ("CaloCellContainer#SCell",                                'ESD AODFULL', 'L1'),

    # Particle Flow Objects, for assessing performance with ITk (and perhaps HGTD)
    ('xAOD::FlowElementContainer#HLT_ftfChargedParticleFlowObjects', 'BS ESD', 'Jet'),
    ('xAOD::FlowElementAuxContainer#HLT_ftfChargedParticleFlowObjectsAux.'+'.'.join(cPFOVarsToKeep), 'BS ESD', 'Jet'),
    ('xAOD::FlowElementContainer#HLT_ftfNeutralParticleFlowObjects', 'BS ESD', 'Jet'),
    ('xAOD::FlowElementAuxContainer#HLT_ftfNeutralParticleFlowObjectsAux.'+'.'.join(nPFOVarsToKeep), 'BS ESD', 'Jet'),

]
