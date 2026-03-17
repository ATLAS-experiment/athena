# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

#####################################################################
# Sequence TauIDs
#####################################################################

# List of Tau ID inference algorithms to be executed in each reco sequence
# Since the TrigTauRecMerged reco (TES, track association, variable calculation, etc.) is very fast,
# we split the reconstruction according to the primary ID algorithm to be used, to avoid running unnecesary long inferences
# The configuration for each TauID algorithm is contained in the flags.Trigger.Offline.Tau.<TauID> subdirectory

def getPrecisionSequenceTauIDs(flags, precision_sequence: str) -> list[str]:
    '''Get the list of TauIDs for each HLT tau trigger sequence'''
    tau_ids = {
        'MVA': ['GNTau', 'MesonCuts', 'GNTauDev1'],
        'LLP': ['RNNLLP'],
        'LRT': ['RNNLLP'],
    }

    # Additional Tau ID algorithms to run ONLY if we're using the MC (or Dev) menu
    mc_tau_ids = {
        'MVA': ['DeepSet'],
    }

    # Additional Tau ID algorithms to run ONLY if we're using the Dev menu
    dev_tau_ids = {
        'MVA': [],
    }

    ret = tau_ids[precision_sequence]
    if any(pfx in flags.Trigger.triggerMenuSetup for pfx in ['MC_', 'Dev_']) and precision_sequence in mc_tau_ids:
        ret += mc_tau_ids[precision_sequence]
    if 'Dev_' in flags.Trigger.triggerMenuSetup and precision_sequence in dev_tau_ids:
        ret += dev_tau_ids[precision_sequence]
    return ret


#####################################################################
# This file contains helper functions for the Tau Trigger signature
#####################################################################

# The following functions are only required while  we still have triggers
# with the RNN/DeepSet naming scheme in the Menu (e.g. mediumRNN_tracktwoMVA/LLP)
rnn_wps = ['verylooseRNN', 'looseRNN', 'mediumRNN', 'tightRNN']
noid_selections = ['perf', 'idperf']
meson_selections = ['kaonpi1', 'kaonpi2', 'dipion1', 'dipion2', 'dipion3', 'dipion4', 'dikaonmass', 'singlepion']

def getChainIDConfigName(flags, chainPart) -> str:
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

    # Sort ID names from longest to shortest, to check for a full match
    tau_ids = sorted(list(flags.Trigger.Offline.Tau), key=len, reverse=True)
    for tau_id in tau_ids:
        if sel.endswith(tau_id): return tau_id

    # Remap names (e.g. DS -> DeepSet)
    name_mapping: dict[str, str] = {'DS': 'DeepSet', 'GNT': 'GNTau'}
    name_mapping = dict(sorted(name_mapping.items(), key=lambda p: len(p[0]), reverse=True))
    for short_name, long_name in name_mapping.items():
        if sel.endswith(short_name): return long_name

    return sel


def getChainSequenceConfigName(chainPart) -> str:
    '''Get the HLT Tau signature sequence name (e.g. ptonly, tracktwo, trackLRT, etc...)'''
    return chainPart['reconstruction']


def getChainPrecisionSeqName(chainPart) -> str:
    '''
    Get the HLT Tau Precision sequence name suffix.
    This is also used for the HLT_TrigTauRecMerged_... and HLT_tautrack_... EDM collection names.
    '''
    ret = chainPart['reconstruction']

    # Support for the Legacy trigger names:
    if ret == 'tracktwoMVA': return 'MVA'
    elif ret == 'tracktwoLLP': return 'LLP'
    elif ret == 'trackLRT': return 'LRT'
    
    return ret


def useBuiltInTauJetRNNScore(tau_id: str, precision_sequence: str) -> bool:
    '''Check if the TauJet's built-in RNN score and WP variables have to be used, instead of the decorator-based variables'''
    # Support for "legacy" algorithms, where the scores are stored in the built-in TauJet aux variables
    if (tau_id == 'DeepSet' and precision_sequence == 'MVA') or (tau_id == 'RNNLLP' and precision_sequence in ['LLP', 'LRT']):
        return True

    return False


def getTauIDScoreVariables(tau_id: str, precision_sequence: str) -> tuple[str, str]:
    '''Return the (score, score_sig_trans) variable name pair for a given TauID/Sequence configuration'''
    # Support for "legacy" algorithms, where the scores are stored in the built-in TauJet aux variables
    if useBuiltInTauJetRNNScore(tau_id, precision_sequence):
        return ('RNNJetScore', 'RNNJetScoreSigTrans')

    return (f'{tau_id}_Score', f'{tau_id}_ScoreSigTrans')
