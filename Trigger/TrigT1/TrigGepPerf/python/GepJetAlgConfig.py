# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepJetAlgCfg(
        flags,
        name, 
        jetAlgName,
        caloClustersKey,
        outputJetsKey,
        wta_seed_cleaning_name='TwoPass',
        wta_min_cluster_et=2.0, # In GeV
        wta_min_seed_et=5.0,
        wta_inf_buffer=False,
        wta_max_const_n=205,
        wta_max_seed_sorting_n=50,
        wta_jet_dR2=0.16,
        wta_block_n=4,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()


    assert jetAlgName in ('ModAntikT', 'Cone', 'WTACone')
    if jetAlgName == 'WTACone':
        assert wta_seed_cleaning_name in ('Baseline', 'TwoPass')
        assert wta_block_n in (1, 4)
        if wta_inf_buffer:
            wta_max_const_n = 9999
            wta_max_seed_sorting_n = 9999

    alg = CompFactory.GepJetAlg(name,
                                jetAlgName=jetAlgName,
                                caloClustersKey=caloClustersKey,
                                outputJetsKey=outputJetsKey,
                                WTAConstEtCut=wta_min_cluster_et,
                                WTASeedEtCut=wta_min_seed_et,
                                WTAMaxConstN=wta_max_const_n,
                                WTAMaxSeedSortingN=wta_max_seed_sorting_n,
                                WTAJet_dR2=wta_jet_dR2,
                                WTASeedCleaningName=wta_seed_cleaning_name,
                                WTABlockN=wta_block_n)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
