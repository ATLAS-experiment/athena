#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    from AthenaConfiguration.Enums import LHCPeriod
    if getattr(flags.Trigger, "doOnlineNavigationCompactification", True) is False:
        default_key = 'HLTNav_Summary' # This can only be for Run 3(+), and is a very niche case of exporting un-compacted navigation collections for advanced debugging
    elif getattr(flags.Trigger, "doEDMVersionConversion", False) is True and getattr(flags.GeoModel, "Run", LHCPeriod.Run3) <= LHCPeriod.Run2:
        default_key = 'HLTNav_R2ToR3Summary' # This can only be for Run 2 navigation conversion.
    else:
        default_key = 'HLTNav_Summary_OnlineSlimmed' # This is the default expected key for the navigation container within the serialised HLT payload inside Run 3(+) RAW files.
    to_return = default_key

    for key in possible_keys:
        if key in flags.Input.Collections:
            to_return = key
            break

    msg = logging.getLogger('getRun3NavigationContainerFromInput')
    msg.info('Returning %s as the Run 3 trigger navigation collection to read in this job.', to_return)

    # Double check 'possible_keys' is kept up to date
    if to_return not in possible_keys:
        msg.error('Must add %s to the "possible_keys" array!', to_return)

    return to_return
