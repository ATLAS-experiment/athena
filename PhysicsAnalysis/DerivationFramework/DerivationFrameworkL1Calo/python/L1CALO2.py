# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# L1CALO2.py
# Contact: paul.daniel.thompson@cern.ch
# Component accumulator version (L1Calo legacy unthinned towers)
# IMPORTANT: this is NOT an AOD based derived data type but one built
# during reconstruction from HITS or RAW. It consequently has to be 
# run from Reco_tf  
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Main config
def L1CALO2Cfg(flags):
    """Main config fragment for L1CALO2"""
    acc = ComponentAccumulator()
    from AthenaCommon.Logging import logging
    log = logging.getLogger('RAWtoALL')
    log.info('****************** L1CALO2 *****************')

    from DerivationFrameworkL1Calo.L1CALOCore import L1CALOCoreCfg 
    L1CALO2 = L1CALOCoreCfg(flags, deriv="L1CALO2")
    acc.merge(L1CALO2)

    return acc


