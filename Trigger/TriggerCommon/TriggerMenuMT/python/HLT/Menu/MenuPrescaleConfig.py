# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.MenuPrescaleSet import AutoPrescaleSetGen

'''
For simple use-cases, automatic prescale sets can be generated according to the following rules.

To apply them, add the "_{prescale_set}_prescale" prefix to the provided menu name.
E.g. `flags.Trigger.triggerMenuSetup='MC_pp_run3_v1_NoBulkMCProd_prescale'`.

This is NOT meant to be used in data-taking, only for development tests and standard reprocessings/MC productions.

For any more complex prescale sets, refer to the `TrigMenuRulebook` (used to create PS sets for data-taking), 
located at: https://gitlab.cern.ch/atlas-trigger-menu/TrigMenuRulebook/
'''

primary_groups = [
    'Primary:PhaseI', 'Primary:Legacy', 'Primary:L1Muon', 'Primary:CostAndRate',
    'Support:TagAndProbe', 'Support:PhaseITagAndProbe', 'Support:LegacyTagAndProbe',
]

# The following predefined filters get progressibly looser
menu_prescale_set_gens = {
    # Primary (always unprescaled) triggers only, disabling unused L1 triggers.
    'PrimaryL1PS': AutoPrescaleSetGen(enable_groups=primary_groups, disable_unused_l1_triggers=True),

    # Primary (always unprescaled) triggers only.
    'Primary': AutoPrescaleSetGen(enable_groups=primary_groups),

    # Excludes a handful of triggers not to be used in reprocessing jobs, mostly due to CPU cost.
    # Additionally, PS some high-rate low-mu triggers that are always prescaled in standard pp data-taking.
    'HLTReprocessing': AutoPrescaleSetGen(
        disable_groups=['PS:NoHLTRepro'],
        chain_prescales={
            'HLT_cosmic_id_L1MU3V_EMPTY': 10,
            'HLT_cosmic_id_L1MU8VF_EMPTY': 10,
        }
    ),

    # Adds more specialized triggers for trigger performance and/or CP studies, possibly run heavily PSed in data.
    'TriggerValidation': AutoPrescaleSetGen(disable_groups=['PS:NoHLTRepro', 'PS:NoTrigVal']),

    # Triggers for generic CP and physics analysis work.
    'BulkMCProd': AutoPrescaleSetGen(disable_groups=['PS:NoHLTRepro', 'PS:NoTrigVal', 'PS:NoBulkMCProd']),
}
