# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# L1CALO1.py
# Contact: paul.daniel.thompson@cern.ch
# Component accumulator version (L1Calo legacy thinned towers)
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be 
# run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Main config
def L1CALO1Cfg(flags):
    """Main config fragment for L1CALO1"""
    acc = ComponentAccumulator()
    from AthenaCommon.Logging import logging
    log = logging.getLogger('RAWtoALL')
    log.info('****************** L1CALO1 *****************')

    from DerivationFrameworkL1Calo.L1CALOCore import L1CALOCoreCfg 
    L1CALO1 = L1CALOCoreCfg(flags, deriv="L1CALO1")
    acc.merge(L1CALO1)

    return acc


