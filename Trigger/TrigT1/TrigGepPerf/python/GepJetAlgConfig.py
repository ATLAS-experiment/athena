# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from enum import Enum

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
        wta_jet_dR=0.4,
        wta_block_n=4,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()


    if jetAlgName not in ('ModAntikT', 'Cone', 'WTACone'):
        raise ValueError("jetAlgName must be one of ModAntikT, Cone, WTACone")
    if jetAlgName == 'WTACone':
        if wta_seed_cleaning_name not in ('Baseline', 'TwoPass'):
            raise ValueError("wta_seed_cleaning_name must be Baseline or TwoPass")
        if wta_block_n not in (1, 4):
            raise ValueError("wta_block_n must be 1 or 4")
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
                                WTAJet_dR=wta_jet_dR,
                                WTASeedCleaningName=wta_seed_cleaning_name,
                                WTABlockN=wta_block_n)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)

    return cfg


# JetTaggerLRJ seed / constituent source options. These mirror the C++ enums
# Gep::JetTaggerSeedSource and Gep::JetTaggerConstSource (JetTaggerLRJMaker.h);
# keep the string values in sync with them.
class _StrEnum(Enum):
    @classmethod
    def to_list(cls):
        return [member.value for member in cls]


class LRJSeedSourceType(_StrEnum):
    WTACone = "WTACone"
    jFexSRJ = "jFexSRJ"
    gFexSRJ = "gFexSRJ"


class LRJConstSourceType(_StrEnum):
    Towers  = "Towers"
    WTACone = "WTACone"


# ---------------------------------------------------------------------------
# JetTaggerLRJ presets, with version-dependent settings.
# GepJetTaggerLRJAlgCfg. Keyed by GepJetAlg property name.
#   2 = BasicV2    (reclusters jets, no substructure, seed-opt disabled)
#   3 = AdvancedV3 (tower-level granularity, full substructure)
# ---------------------------------------------------------------------------
_LRJ_PRESETS = {
    2: {
        'LRJDSearch':                  0.001,  # rMergeCut (seed-pos-opt effectively disabled)
        'LRJMaxObjectsConsidered':     8,
        'LRJEtaBitLength':             10,
        'LRJPhiBitLength':             9,
        'LRJNumSubjetsLength':         0,      # no substructure variables computed
        'LRJNSubjetinessBitLength':    0,
        'LRJMassApproxBitLength':      0,
        'LRJPsiRBitLength':            0,
        'LRJEnableOverlapRemoval':     False,
        'LRJMinEtSeedPosOptimization': False,
    },
    3: {
        'LRJDSearch':                  2.0,
        'LRJMaxObjectsConsidered':     512,    # raised 128->512 so the E_T>2 GeV tower cut (not this cap) bounds the input; 128 kept elsewhere for latency estimation
        'LRJEtaBitLength':             7,
        'LRJPhiBitLength':             6,
        'LRJNumSubjetsLength':         2,
        'LRJNSubjetinessBitLength':    8,
        'LRJMassApproxBitLength':      8,
        'LRJPsiRBitLength':            8,
        'LRJEnableOverlapRemoval':     True,
        'LRJMinEtSeedPosOptimization': True,
    },
}


def GepJetTaggerLRJAlgCfg(flags, name, **kwargs):

    # ---- Configure GepJetAlg in JetTaggerLRJ mode ----

    cfg = ComponentAccumulator()

    # This wrapper always drives GepJetAlg in JetTaggerLRJ mode.
    kwargs['jetAlgName'] = 'JetTaggerLRJ'
    kwargs['EnableLRJMaker'] = True

    # ---- preset selector: 2 = BasicV2, 3 = AdvancedV3 ----
    algo_version = kwargs.setdefault('LRJAlgoVersion', 3)
    if algo_version not in (2, 3):
        raise ValueError("LRJAlgoVersion must be 2 (BasicV2) or 3 (AdvancedV3)")
    for prop, value in _LRJ_PRESETS[algo_version].items():
        kwargs.setdefault(prop, value)

    # ---- seed / constituent sources ----
    kwargs.setdefault('LRJSeedSource', LRJSeedSourceType.WTACone.value)   # WTACone | jFexSRJ | gFexSRJ
    kwargs.setdefault('LRJConstSource', LRJConstSourceType.Towers.value)  # Towers | WTACone
    kwargs.setdefault('LRJWTAConeSeedsKey', '')              # Required if LRJSeedSource == 'WTACone'
    kwargs.setdefault('jFexSRJetRoIs', 'L1_jFexSRJetRoISim')
    kwargs.setdefault('LRJgFexSRJetRoIs', 'L1_gFexSRJetRoISim')

    # ---- version-independent settings (identical across presets) ----
    kwargs.setdefault('LRJJetR', 1.1)                        # r2Cut = LRJJetR**2 = 1.21
    kwargs.setdefault('LRJNSeedsInput', 10)
    kwargs.setdefault('LRJNProtoSeeds', 6)
    kwargs.setdefault('LRJNSeedsOutput', 2)
    kwargs.setdefault('LRJEtBitLength', 13)
    kwargs.setdefault('LRJDeltaRLutLength', 8)
    kwargs.setdefault('LRJEnableEtWeightedMidpoint', False)  # geometric midpoint
    kwargs.setdefault('LRJSubjetEtThresholdGeV', 25.0)
    kwargs.setdefault('LRJMinEtSeedPosOptCutGeV', 25.0)
    kwargs.setdefault('LRJSeedEtCutGeV', 5.0)                # reserved; not yet applied
    kwargs.setdefault('LRJConstEtCutGeV', 2.0)               # reserved; not yet applied

    # ---- digitization: physical ranges ----
    kwargs.setdefault('LRJPhiMin', -3.2)
    kwargs.setdefault('LRJPhiMax', 3.2)
    kwargs.setdefault('LRJEtaMin', -4.85)
    kwargs.setdefault('LRJEtaMax', 4.95)
    kwargs.setdefault('LRJEtMin', 0.0)
    kwargs.setdefault('LRJEtMax', 1024.0)
    kwargs.setdefault('LRJMassApproxMax', 512.0)
    kwargs.setdefault('LRJInputEtToGeV', 1.0e-3)

    # ---- output toggles ----
    kwargs.setdefault('LRJWriteSubstructure', True)
    kwargs.setdefault('LRJWriteSubjetKinematics', True)
    kwargs.setdefault('LRJWriteConstituentIndices', True)

    # Ensure that configuration is within possible values for the algorithm
    if kwargs['LRJSeedSource'] not in LRJSeedSourceType.to_list():
        raise ValueError(f"LRJSeedSource must be one of {LRJSeedSourceType.to_list()}")
    if kwargs['LRJConstSource'] not in LRJConstSourceType.to_list():
        raise ValueError(f"LRJConstSource must be one of {LRJConstSourceType.to_list()}")
    if kwargs['LRJSeedSource'] == LRJSeedSourceType.WTACone.value \
            and not kwargs['LRJWTAConeSeedsKey']:
        raise ValueError("LRJWTAConeSeedsKey must be set when LRJSeedSource == 'WTACone'")

    alg = CompFactory.GepJetAlg(name, **kwargs)

    cfg.addEventAlgo(alg)

    return cfg