#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# AccumulatorCache is not available in AnalysisBase so function without cache
# created first for analysis configs

from AthenaCommon.Logging import logging


# List of all possible keys of the Run 3 navigation summary collection
# in order of verbosity. Want to take the most verbose which is available.
possible_keys = [
    'HLTNav_Summary', # Produced initially online (only the final nodes, all other nodes spread out over many many collections created by trigger framework algs)
    'HLTNav_Summary_OnlineSlimmed', # Produced online, all nodes in one container. Accessible during RAWtoALL, good for T0 monitoring.
    'HLTNav_Summary_ESDSlimmed', # Produced offline in jobs writing ESD. Minimal slimming, good for T0-style monitoring. Equivalent to AOD at AODFULL level.
    'HLTNav_Summary_AODSlimmed', # Produced offline in jobs writing AOD. Minimal slimming in AODFULL mode, good for T0-style monitoring. Slimming applied in AODSLIM mode, good for analysis use, final-features accessible.
    'HLTNav_Summary_DAODSlimmed', # Chain level slimming and IParticle feature-compacting for DAOD. Good for analysis use, final-features four vectors accessible.
    'HLTNav_R2ToR3Summary' # Output of Run 2 to Run 3 navigation conversion procedure. Somewhat equivalent to AODFULL level. Designed to be further reduced to DAODSlimmed level before analysis use.
    ]

def getRun3NavigationContainerFromInput_forAnalysisBase(flags):
    # What to return if we cannot look in the file
    default_key = 'HLTNav_Summary_OnlineSlimmed' if getattr(flags.Trigger, "doOnlineNavigationCompactification", False) else 'HLTNav_Summary'
    to_return = default_key

    if getattr(flags.Trigger, "doEDMVersionConversion", False):
        to_return = 'HLTNav_R2ToR3Summary'
    else:
        for key in possible_keys:
            if key in flags.Input.Collections:
                to_return = key
                break

    msg = logging.getLogger('getRun3NavigationContainerFromInput')
    msg.info('Returning %s as the Run 3 trigger navigation colletion to read in this job.', to_return)

    # Double check 'possible_keys' is kept up to date
    if to_return not in possible_keys:
        msg.error('Must add %s to the "possible_keys" array!', to_return)

    return to_return
