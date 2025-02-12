# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

#####################################################################
# Sequence TauIDs
#####################################################################

# List of Tau ID inference algorithms to be executed in each reco sequence
# Since the TrigTauRecMerged reco (TES, track association, variable calculation, etc.) is very fast,
# we split the reconstruction according to the primary ID algorithm to be used, to avoid running unnecesary long inferences
# The configuration for each TauID algorithm is contained in the flags.Trigger.Offline.Tau.<TauID> subdirectory

def getPrecisionSequenceTauIDs(precision_sequence: str) -> list[str]:
    '''Get the list of TauIDs for each HLT tau trigger sequence'''
    tau_ids = {
        'MVA': ['GNTau', 'DeepSet', 'MesonCuts'],
        'LLP': ['RNNLLP'],
        'LRT': ['RNNLLP'],
    }

    return tau_ids[precision_sequence]

#####################################################################
# This file contains helper functions for the Tau Trigger signature
#####################################################################

# The following functions are only required while  we still have triggers
# with the RNN/DeepSet naming scheme in the Menu (e.g. mediumRNN_tracktwoMVA/LLP)
rnn_wps = ['verylooseRNN', 'looseRNN', 'mediumRNN', 'tightRNN']
noid_selections = ['perf', 'idperf']
meson_selections = ['kaonpi1', 'kaonpi2', 'dipion1', 'dipion2', 'dipion3', 'dipion4', 'dikaonmass', 'singlepion']

def getChainIDConfigName(chainPart) -> str:
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


    # Retrieve the TauID name from the selection string
    if sel.startswith('veryloose'): sel = sel.removeprefix('veryloose')
    if sel.startswith('loose'): sel = sel.removeprefix('loose')
    if sel.startswith('medium'): sel = sel.removeprefix('medium')
    if sel.startswith('tight'): sel = sel.removeprefix('tight')
    
    # Remap names (e.g. DS -> DeepSet)
    name_mapping: dict[str, str] = {'DS': 'DeepSet', 'GNT': 'GNTau'}
    if sel in name_mapping: sel = name_mapping[sel]

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
