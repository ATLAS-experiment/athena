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

]
