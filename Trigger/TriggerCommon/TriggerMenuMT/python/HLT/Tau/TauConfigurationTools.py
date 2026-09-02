# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from typing import Any

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

# This file contains helper functions for the Tau Trigger signature configuration


#####################################################################
# Global helper functions
#####################################################################
def getMenuAlgs(
    flags: AthConfigFlags, 
    key: str | None = None, 
    alt_key: str | None = None,
    algs: dict[str, list[str]] | list[str] | None = None, 
    mc_algs: dict[str, list[str]] | list[str] | None = None, 
    dev_algs: dict[str, list[str]] | list[str] | None = None,
) -> list[str]:
    '''Get the list of algorithms for a specific menu key; if not found, the alternate key will be tried if provided.'''
    def _getAlgs(key: str | None):
        if algs is None: ret = None
        elif isinstance(algs, dict): ret = algs[key] if key in algs else None
        else: ret = algs # list
        if any(pfx in flags.Trigger.triggerMenuSetup for pfx in ['MC_', 'Dev_']) and mc_algs and (isinstance(mc_algs, list) or key in mc_algs):
            if ret is None: ret = []
            ret += mc_algs[key] if isinstance(mc_algs, dict) else mc_algs
        if 'Dev_' in flags.Trigger.triggerMenuSetup and dev_algs and (isinstance(dev_algs, list) or key in dev_algs):
            if ret is None: ret = []
            ret += dev_algs[key] if isinstance(dev_algs, dict) else dev_algs
        return ret

    ret_algs = _getAlgs(key)
    if ret_algs is None and alt_key is not None:
        ret_algs = _getAlgs(alt_key)
    return ret_algs
    


def useBuiltInTauJetRNNScore(tau_id: str) -> bool:
    '''Check if the TauJet's built-in RNN score and WP variables have to be used, instead of the decorator-based variables'''
    # Support for "legacy" algorithms, where the scores are stored in the built-in TauJet aux variables
    return tau_id in ['DeepSet', 'RNNLLP']


def getTauIDScoreVariables(tau_id: str) -> tuple[str, str]:
    '''Return the (score, score_sig_trans) variable name pair for a given TauID/Sequence configuration'''
    # Support for "legacy" algorithms, where the scores are stored in the built-in TauJet aux variables
    if useBuiltInTauJetRNNScore(tau_id): return ('RNNJetScore', 'RNNJetScoreSigTrans')

    return (f'{tau_id}_Score', f'{tau_id}_ScoreSigTrans')


@AccumulatorCache  # called many times and looping over flags is slow
def getTauIDAlgorithm(flags: AthConfigFlags, selection: str,
                      name_mapping: tuple[tuple[str, str], ...] | None = None) -> str:

    # Sort ID names from longest to shortest, to check for a full match
    tau_ids = sorted(flags.Trigger.Offline.Tau, key=len, reverse=True)
    for tau_id in tau_ids:
        if selection.endswith(tau_id): return tau_id

    # Remap names (e.g. DS -> DeepSet)
    if name_mapping:
        name_mapping = sorted(name_mapping, key=lambda p: len(p[0]), reverse=True)
        for short_name, long_name in name_mapping:
            if selection.endswith(short_name): return long_name
    
    return selection



#####################################################################
# CaloHits sequence algorithms
#####################################################################

def getHitZAlgs(flags: AthConfigFlags, precision_sequence: str, alt_precision_sequence: str | None = None) -> list[str]:
    '''
    Get the list of HitZ algorithms for the CaloHits reco sequence.
    The configuration for each algorithm is contained in flags.Trigger.Offline.Tau.<alg>.
    '''
    return getMenuAlgs(
        flags,
        key=precision_sequence,
        alt_key=alt_precision_sequence,

        # Default HitZ algorithms to run in all menus
        algs=['HitZ'],

        # Additional HitZ algorithms to run ONLY if we're using the MC (or Dev) menu
        mc_algs={},

        # Additional HitZ algorithms to run ONLY if we're using the Dev menu
        dev_algs={},
    )


def getHitZConfig(flags: AthConfigFlags, chainPart: dict[str, Any]) -> tuple[str, float] | None:
    '''
    Get the HLT HitZ configuration tuple: (algorithm name, sigma cut value in mm)
    '''
    if not chainPart['hitz']: return None

    import re
    # Match strings of the form: '10mmX5mmHitZ', '5mmHitZ', 'HitZ', etc...
    match = re.match(r'((?P<sigma>(\d|p)+)mm)?(X(\d|p)+mm)?(?P<alg>.+)', chainPart['hitz'])
    if match:
        alg = match.group('alg')

        alg_flags = getattr(flags.Trigger.Offline.Tau, alg, None)
        if alg_flags is None:
            raise ValueError(f'HitZ algorithm "{alg}" configuration not found in flags.Trigger.Offline.Tau.{alg}')

        sigma = match.group('sigma')
        if sigma is None: sigma = alg_flags.DefaultMaxZ0Sigma # mm (default value)
        else: sigma = float(sigma.replace('p', '.')) # mm

        return (alg, sigma)
    
    raise ValueError(f'Invalid HitZ configuration string: {chainPart["hitz"]}')


def getHitZVariables(alg: str) -> tuple[str, str]:
    '''Return the (z, sigma) variable name pair for a given HitZ algorithm'''
    return (f'{alg}_z0', f'{alg}_z0_sigma')


def getCaloHitsPreselAlgs(flags: AthConfigFlags, precision_sequence: str, alt_precision_sequence: str | None = None) -> list[str]:
    '''
    Get the list of CaloHits preselection TauID inferences to be executed for the CaloHits reco sequence.
    The configuration for each algorithm is contained in flags.Trigger.Offline.Tau.<alg>.
    '''
    return getMenuAlgs(
        flags,
        key=precision_sequence,
        alt_key=alt_precision_sequence,

        # Default inferences to run in all menus
        algs=[],

        # Additional inferences to run ONLY if we're using the MC (or Dev) menu
        mc_algs=[],

        # Additional inferences to run ONLY if we're using the Dev menu
        dev_algs=[],
    )


def getChainCaloHitsPreselConfigName(flags: AthConfigFlags, chainPart: dict[str, Any]) -> str:
    '''Clean the CaloHits preselection configuration for a chainPart dict'''
    sel = chainPart['calohitsPresel']

    if not sel or sel == 'idperfCHP': return 'idperf' # No preselection

    return getTauIDAlgorithm(
        flags,
        sel,
        name_mapping=(('CHTP', 'GNCaloHitsTauPresel')),
    )


def getChainCaloHitsSeqName(chainPart: dict[str, Any]) -> str | None:
    '''Get the HLT Tau CaloHits sequence name suffix'''
    if not chainPart['hitz'] and not chainPart['calohitsPresel']: return None

    parts = []

    # HitZ RoI updating selection
    if chainPart['hitz']: parts.append(chainPart['hitz'])

    if not parts: parts = ['CaloHitsBase']
    return '_'.join(parts)



#####################################################################
# Precision sequence TauIDs
#####################################################################

def getPrecisionSequenceTauIDs(flags: AthConfigFlags, precision_sequence: str, alt_precision_sequence: str | None = None) -> list[str]:
    '''
    Get the list of precision TauID inferences to be executed for each HLT tau trigger reco sequence
    The configuration for each algorithm is contained in flags.Trigger.Offline.Tau.<alg>.
    '''
    return getMenuAlgs(
        flags,
        key=precision_sequence,
        alt_key=alt_precision_sequence,

        # Default Tau ID algorithms to run in all menus
        algs={
            'MVA': ['GNTau', 'MesonCuts', 'GNTauDev1'],
            'EM':  ['GNTau', 'MesonCuts', 'GNTauDev1'],
            'LLP': ['RNNLLP'],
            'LRT': ['RNNLLP'],
        },

        # Additional Tau ID algorithms to run ONLY if we're using the MC (or Dev) menu
        mc_algs={
            'MVA': ['DeepSet'],
            'EM':  ['DeepSet'],
        },

        # Additional Tau ID algorithms to run ONLY if we're using the Dev menu
        dev_algs={
        }
    )


# The following functions are only required while  we still have triggers
# with the RNN/DeepSet naming scheme in the Menu (e.g. mediumRNN_tracktwoMVA/LLP)
rnn_wps = ['verylooseRNN', 'looseRNN', 'mediumRNN', 'tightRNN']
noid_selections = ['perf', 'idperf']
meson_selections = ['kaonpi1', 'kaonpi2', 'dipion1', 'dipion2', 'dipion3', 'dipion4', 'dikaonmass', 'singlepion']

def getChainIDConfigName(flags: AthConfigFlags, chainPart: dict[str, Any]) -> str:
    '''Clean the ID configuration for a chainPart dict'''
    sel = chainPart['selection']

    # Support for the Legacy trigger names:
    if chainPart['reconstruction'] == 'tracktwoMVA':
        if sel in rnn_wps:
            return 'DeepSet'
        elif sel in meson_selections:
            return 'MesonCuts'
    elif chainPart['reconstruction'] in ['tracktwoLLP', 'trackLRT'] and sel in rnn_wps:
        return 'RNNLLP'

    return getTauIDAlgorithm(
        flags,
        sel,
    )

    return sel


def getChainPrecisionSeqName(chainPart: dict[str, Any], include_calohits_seq_name: bool = False) -> str:
    '''
    Get the HLT Tau Precision sequence name suffix.
    This is also used for the HLT_TrigTauRecMerged_... and HLT_tautrack_... EDM collection names.
    '''
    ret = chainPart['reconstruction']

    # Support for the Legacy trigger names:
    if ret == 'tracktwoMVA': return 'MVA'
    elif ret == 'tracktwoLLP': return 'LLP'
    elif ret == 'trackLRT': return 'LRT'

    if include_calohits_seq_name:
        calohits_seq = getChainCaloHitsSeqName(chainPart)
        ret += f'_{calohits_seq}' if calohits_seq else ''
    
    return ret



#####################################################################
# Global Tau menu sequence
#####################################################################

def getChainSequenceConfigName(chainPart: dict[str, Any]) -> str:
    '''Get the HLT Tau signature global menu sequence name (e.g. ptonly, tracktwo, trackLRT, etc...)'''
    name = []

    if chainPart['hitz'] or chainPart['calohitsPresel']:
        name.append('CaloHits')

    name.append(chainPart['reconstruction'])

    return '_'.join(name)

