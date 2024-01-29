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
 
]
